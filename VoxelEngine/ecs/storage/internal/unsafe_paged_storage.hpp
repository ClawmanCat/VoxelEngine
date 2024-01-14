#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/debug/assert.hpp>

#include <concepts>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <bit>


namespace ve::ecs::detail {
    /** Mixin types for use with unsafe_paged_storage. */
    namespace paged_storage_mixins {
        /** Concept for mixins used to initialize an empty page of an unsafe_paged_storage. */
        template <typename T, typename VT> concept page_initializer = std::is_invocable_v<T, VT*, VT*>;
        /** Concept for mixins used to destroy a value within an unsafe_paged_storage. */
        template <typename T, typename VT> concept value_destructor = std::is_invocable_v<T, VT*>;


        /** @ref page_initializer which leaves the page uninitialized. This is the default behaviour. */
        template <typename T> struct noop_initializer {
            constexpr void operator()(T* begin, T* end) const { }
        };

        /** @ref page_initializer which initializes the page with a constant value. */
        template <typename T, T Value> struct constant_initializer {
            constexpr void operator()(T* begin, T* end) const { std::uninitialized_fill(begin, end, Value); }
        };

        /** @ref value_destructor which calls the destructor on erased values. This is the default behaviour. */
        template <typename T> struct default_destructor {
            constexpr void operator()(T* value) const { value->~T(); }
        };

        /** @ref value_destructor which overwrites erased values with a default value. Requires trivial destructibility to prevent UB. */
        template <typename T, T Value> requires std::is_trivially_destructible_v<T> struct overwrite_destructor {
            constexpr void operator()(T* value) const { (*value) = Value; }
        };
    }


    /**
     * Implementation used for paged storage in sparse sets and component pools.
     * This container will invoke undefined behaviour when used improperly and should not be used as a general purpose container.
     * @tparam T The type of object stored within this container.
     * @tparam PageSize The size of each page in number of Ts (i.e. the size in bytes is PageSize * sizeof(T)).
     * @tparam PageInitializerMixin Function object invoked to initialize a new page. See @ref paged_storage_mixins.
     * @tparam ValueDestructorMixin Function object invoked to destroy a value. See @ref paged_storage_mixins.
     */
    template <
        typename T,
        std::size_t PageSize,
        paged_storage_mixins::page_initializer<T> PageInitializerMixin = paged_storage_mixins::noop_initializer<T>,
        paged_storage_mixins::value_destructor<T> ValueDestructorMixin = paged_storage_mixins::default_destructor<T>
    > requires (std::popcount(PageSize) == 1) // Page size must be a power of two
    class unsafe_paged_storage {
    public:
        constexpr static inline bool trivially_destructible = std::is_trivially_destructible_v<T>;
        constexpr static inline bool is_copyable            = std::is_trivially_copyable_v<T> && std::is_copy_assignable_v<PageInitializerMixin> && std::is_copy_assignable_v<ValueDestructorMixin>;
        constexpr static inline bool is_movable             = std::is_move_assignable_v<PageInitializerMixin> && std::is_move_assignable_v<ValueDestructorMixin>;


        explicit unsafe_paged_storage(PageInitializerMixin page_initializer = PageInitializerMixin { }, ValueDestructorMixin value_destructor = ValueDestructorMixin { }) :
            page_initializer(std::move(page_initializer)),
            value_destructor(std::move(value_destructor))
        { }


        unsafe_paged_storage(unsafe_paged_storage&& o) noexcept requires is_movable { *this = std::move(o); }

        unsafe_paged_storage& operator=(unsafe_paged_storage&& o) noexcept requires is_movable {
            pages            = std::move(o.pages);
            size             = std::exchange(o.size, 0);
            page_initializer = std::move(o.page_initializer);
            value_destructor = std::move(o.value_destructor);

            return *this;
        }


        unsafe_paged_storage(const unsafe_paged_storage& o) requires is_copyable { *this = o; }

        unsafe_paged_storage& operator=(const unsafe_paged_storage& o) requires is_copyable {
            pages.reserve(o.pages.size());

            for (const auto& p : o.pages) {
                pages.push_back(p ? std::make_unique<page>(*p) : nullptr);
            }


            size             = o.size;
            page_initializer = o.page_initializer;
            value_destructor = o.value_destructor;


            return *this;
        }


        ~unsafe_paged_storage(void) {
            VE_DEBUG_ASSERT(
                std::is_trivially_destructible_v<T> || size == 0,
                "Destroyed unsafe_paged_storage with remaining elements of non-trivially destructible type."
            );
        }


        /**
         * Emplaces a new object of type T into this container at the given index. A new page is created if necessary.
         * @warning It is the responsibility of the caller to ensure no object already exists at this index!
         * @param index The index to emplace the new object at.
         * @param args Constructor arguments to construct the object with.
         * @return A reference to the newly constructed object.
         */
        template <typename... Args> T& emplace(std::size_t index, Args&&... args) {
            const auto [page, offset] = locate_slot(index);

            // Create page if one does not exist for the given slot.
            if (pages.size() <= page) pages.resize(page + 1);

            VE_MUTE_WARNING(
                (GCC, clang),
                -Wcast-align,
                if (!pages[page]) {
                    pages[page] = std::make_unique<struct page>();

                    std::invoke(
                        page_initializer,
                        (T*) std::assume_aligned<alignof(T)>(std::begin(pages[page]->buffer)),
                        (T*) std::assume_aligned<alignof(T)>(std::end(pages[page]->buffer))
                    );
                }
            )

            // Construct element on page and increment counters.
            T* memory = get(*this, page, offset);
            new(memory) T { fwd(args)... };

            ++pages[page]->count;
            ++size;

            // Return constructed element.
            return *memory;
        }


        /**
         * Erases the object stored in this container at the given index. The page containing the object is deleted if it is empty after this operation.
         * @warning It is the responsibility of the caller to ensure an object actually exists at this index!
         * @param index The index of the object to destroy.
         */
        void erase(std::size_t index) {
            const auto [page, offset] = locate_slot(index);

            // Destruct element and decrement counters.
            T* memory = get(*this, page, offset);
            std::invoke(value_destructor, memory);

            --pages[page]->count;
            --size;

            // Destroy page if it is now empty and shrink the storage if there are no more occupied pages after this one.
            if (pages[page]->count == 0) {
                pages[page] = nullptr;
                while (!pages.empty() && pages.back() == nullptr) pages.pop_back();
            }
        }


        /**
         * Clears the storage. This can only be done if contained objects are trivially destructible.
         * For non-trivially-destructible objects, call erase() for every object to clear the storage.
         */
        void clear(void) requires std::is_trivially_destructible_v<T> {
            pages.clear();
            size = 0;
        }


        /** Returns true if the container contains a page for the given index. */
        bool has_page_for(std::size_t index) const {
            const auto [page, offset] = locate_slot(index);
            return pages.size() > page && pages[page];
        }


        /**
         * Returns a reference to the object at the given index.
         * @warning It is the responsibility of the caller to ensure an object actually exists at this index!
         */
        T&       operator[](std::size_t index)       { const auto [page, offset] = locate_slot(index); return *get(*this, page, offset); }
        /** @copydoc operator[] */
        const T& operator[](std::size_t index) const { const auto [page, offset] = locate_slot(index); return *get(*this, page, offset); }


        VE_GET_VALS(size);
        VE_GET_MREFS(page_initializer, value_destructor);
        VE_WRAP_RT_CONST_MEM_FNS(pages, shrink_to_fit);
    private:
        struct page {
            alignas(T) u8 buffer[sizeof(T) * PageSize];
            smallest_integral_type_for<PageSize> count = 0;
        };

        std::vector<unique<page>> pages;
        std::size_t size = 0;

        VE_NO_UNIQUE_ADDRESS PageInitializerMixin page_initializer;
        VE_NO_UNIQUE_ADDRESS ValueDestructorMixin value_destructor;


        /** Returns a pair [page, offset] indicating the location at which the element with the given index is stored. */
        [[nodiscard]] constexpr static std::pair<std::size_t, std::size_t> locate_slot(std::size_t index) {
            constexpr std::size_t exponent = std::countr_zero(PageSize);

            std::size_t page   = index >> exponent;
            std::size_t offset = index - (page * PageSize);

            return { page, offset };
        }


        /** Retrieves the object at the given page and offset. Returns const based on the type of 'Self'. */
        template <typename Self> [[nodiscard]] constexpr static auto* get(Self& self, std::size_t page, std::size_t offset) {
            using pointer_t = std::conditional_t<std::is_const_v<Self>, const T*, T*>;

            VE_MUTE_WARNING(
                (GCC, clang),
                -Wcast-align,
                return (pointer_t) std::assume_aligned<alignof(T)>(&(self.pages[page]->buffer[sizeof(T) * offset]));
            )
        }
    };
}