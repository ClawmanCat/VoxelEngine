#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>
#include <VoxelEngine/ecs/storage/internal/unsafe_paged_storage.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_mixin.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_iterator.hpp>
#include <VoxelEngine/utility/container/container_typedefs.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>
#include <VoxelEngine/utility/meta/value.hpp>

#include <vector>
#include <type_traits>


namespace ve::ecs {
    namespace detail {
        /** @ref unsafe_paged_storage type used as the sparse container for sparse_sets with the given traits. */
        template <entity_traits Traits> using sparse_type_for = detail::unsafe_paged_storage<
            typename Traits::type,
            Traits::page_size,
            detail::paged_storage_mixins::constant_initializer<typename Traits::type, entity_utils<Traits>::tombstone()>,
            detail::paged_storage_mixins::overwrite_destructor<typename Traits::type, entity_utils<Traits>::tombstone()>
        >;
    }




    /**
     * Common base class for read-operations of @ref basic_sparse_set whose type does not depend on the presence of a mixin.
     * Heterogeneous collections of sets may be cast to this base class to provide a common interface for read-operations.
     *
     * @tparam Traits Entity traits for entities stored in this container. Type must fulfil the @ref entity_traits concept.
     */
    template <entity_traits Traits> class basic_sparse_set_common_base {
    public:
        using self_type      = basic_sparse_set_common_base<Traits>;
        using entity_traits  = Traits;
        using entity_type    = typename Traits::type;
        using entity_utils   = entity_utils<Traits>;
        using dense_type     = std::vector<entity_type>;
        using sparse_type    = detail::sparse_type_for<Traits>;
        using tombstone_type = std::conditional_t<Traits::index_stability, std::vector<entity_type>, meta::null_type>;

        // Note: passing const self_type as the iterator parent also removes any iterator dependency on the mixin.
        VE_CONTAINER_ITERATOR_TYPEDEFS(
            entity_type,
            VE_WRAP_TYPENAME(sparse_set_iterator<const self_type>),
            VE_WRAP_TYPENAME(sparse_set_iterator<const self_type>),
            VE_TRUE
        );

        constexpr static inline bool has_index_stability = Traits::index_stability;
        constexpr static inline bool has_tombstones      = has_index_stability;


        VE_CONTAINER_BEGIN_END((this, dense.begin()), (this, dense.end()));


        /** Checks if the set contains an entity with the same index and version. */
        [[nodiscard]] bool contains(entity_type entt) const { return contains_impl<equality_mask>(entt); }
        /** Checks if the set contains an entity with the same index, version, and unassigned bits. */
        [[nodiscard]] bool contains_exact(entity_type entt) const { return contains_impl<exact_equality_mask>(entt); }
        /** Checks if the set contains an entity with the same index. */
        [[nodiscard]] bool contains_any_version(entity_type entt) const { return contains_impl<versionless_equality_mask>(entt); }


        /** Returns an iterator to the entity with the same index and version as the provided entity, or end() if no such entity exists. */
        [[nodiscard]] const_iterator find(entity_type entt) const { return find_impl<equality_mask>(this, entt); }
        /** Returns an iterator to the entity with the same index, version and unassigned bits as the provided entity, or end() if no such entity exists. */
        [[nodiscard]] const_iterator find_exact(entity_type entt) const { return find_impl<exact_equality_mask>(this, entt); }
        /** Returns an iterator to the entity with the same index as the provided entity, or end() if no such entity exists. */
        [[nodiscard]] const_iterator find_any_version(entity_type entt) const { return find_impl<versionless_equality_mask>(this, entt); }


        /** Returns the entity with the given index and version. This method <b>DOES NOT</b> validate the entity is within the set. */
        [[nodiscard]] const_reference operator[](entity_type entt) const { return dense[sparse[entity_utils::index_of(entt)]]; }


        /** Returns the position of the given entity in the dense set. This method <b>DOES NOT</b> validate the entity is within the set. */
        [[nodiscard]] std::size_t get_dense_offset(entity_type entt) const { return sparse[entity_utils::index_of(entt)]; }


        /**
         * Returns a number proportional to the number of operations required to iterate over the set.
         * If the set contains tombstones, this number is larger than size(), since tombstones must still be iterated over.
         */
        [[nodiscard]] std::size_t iteration_complexity(void) const {
            return dense.size();
        }


        /** Returns the number of elements in the set, not counting any tombstones. */
        [[nodiscard]] std::size_t size(void) const {
            if constexpr (has_tombstones) return iteration_complexity() - tombstones.size();
            else return iteration_complexity();
        }


        /** Returns true if the set is empty. */
        [[nodiscard]] bool empty(void) const {
            return size() == 0;
        }


        /** Returns the list of current tombstones. Only available if this set supports tombstones. */
        [[nodiscard]] const tombstone_type& get_tombstones(void) const requires has_tombstones {
            return tombstones;
        }


        VE_GET_CREFS(dense, sparse);
    protected:
        template <typename P> friend class sparse_set_iterator;


        constexpr static inline entity_type equality_mask             = ~entity_utils::unassigned_bits_mask();
        constexpr static inline entity_type exact_equality_mask       = max_value<entity_type>;
        constexpr static inline entity_type versionless_equality_mask = entity_utils::index_mask();


        dense_type dense;
        sparse_type sparse;
        VE_NO_UNIQUE_ADDRESS tombstone_type tombstones;


        /** Must be constructed from derived class */
        basic_sparse_set_common_base(void) = default;
        basic_sparse_set_common_base(const basic_sparse_set_common_base& o) { *this = o; }
        basic_sparse_set_common_base(basic_sparse_set_common_base&& o)      { *this = std::move(o); }
        basic_sparse_set_common_base& operator=(const basic_sparse_set_common_base& o) { VE_COPY_FIELDS(o, dense, sparse, tombstones); return *this; }
        basic_sparse_set_common_base& operator=(basic_sparse_set_common_base&& o)      { VE_MOVE_FIELDS(o, dense, sparse, tombstones); return *this; }


        /** Checks if the set contains an entity whose masked bits compare equal to that of the provided entity. */
        template <entity_type Mask> [[nodiscard]] bool contains_impl(entity_type entt) const {
            const auto index = entity_utils::index_of(entt);

            return (
                sparse.has_page_for(index) &&
                sparse[index] != entity_utils::tombstone() &&
                (dense[sparse[index]] & Mask) == (entt & Mask)
            );
        }


        /** Returns the entity that compares equal according to the provided mask if one exists, or end() otherwise. */
        template <entity_type Mask, typename Self> [[nodiscard]] static auto find_impl(Self* self, entity_type entt) {
            return self->template contains_impl<Mask>(entt)
                   ? self->make_iterator(self->sparse[entity_utils::index_of(entt)])
                   : self->end();
        }


        /** Returns an iterator to the element with the given index in the dense storage. */
        [[nodiscard]] iterator       make_iterator(std::size_t dense_index)       { return iterator       { this, dense.begin() + dense_index }; }
        /** @copydoc make_iterator */
        [[nodiscard]] const_iterator make_iterator(std::size_t dense_index) const { return const_iterator { this, dense.begin() + dense_index }; }
    };




    /**
     * A sparse set container for the storage of ECS entities. This container is based on that of the EnTT library (https://github.com/skypjack/entt).
     *  - This container provides O(1) lookup of entities while providing a contiguous array of entities for fast iteration.
     *  - Entity ID indices should form one or more semi-contiguous value ranges.
     *  - Entity ID indices can and should be re-used by incrementing their version.
     * @tparam Traits Entity traits for entities stored in this container. Type must fulfil the @ref entity_traits concept.
     * @tparam Mixin Optional mixin with callbacks for operations that modify the set. Type must fulfil the @ref sparse_set_mixin_type concept.
     */
    template <entity_traits Traits, sparse_set_mixin_type<Traits> Mixin = no_sparse_set_mixin<Traits>> class basic_sparse_set : public basic_sparse_set_common_base<Traits> {
    public:
        using self_type      = basic_sparse_set<Traits, Mixin>;
        using common_parent  = basic_sparse_set_common_base<Traits>;
        using entity_traits  = Traits;
        using entity_type    = typename Traits::type;
        using entity_utils   = entity_utils<Traits>;
        using dense_type     = std::vector<entity_type>;
        using sparse_type    = detail::sparse_type_for<Traits>;
        using tombstone_type = std::conditional_t<Traits::index_stability, std::vector<entity_type>, meta::null_type>;
        using mixin_type     = Mixin;

        VE_CONTAINER_ITERATOR_TYPEDEFS(
            entity_type,
            VE_WRAP_TYPENAME(sparse_set_iterator<self_type>),
            typename common_parent::const_iterator,
            VE_TRUE
        );

        constexpr static inline bool has_mixin = !std::is_same_v<Mixin, no_sparse_set_mixin<Traits>>;


        basic_sparse_set(void) : common_parent() { set_mixin(mixin_type {}); }
        explicit basic_sparse_set(mixin_type mixin) : common_parent() { set_mixin(std::move(mixin)); }

        /** Construct from a list of entities. */
        explicit basic_sparse_set(std::initializer_list<entity_type> entities, mixin_type mixin = mixin_type { }) : basic_sparse_set(std::move(mixin)) {
            for (const auto entt : entities) insert(entt);
        }

        basic_sparse_set(const basic_sparse_set& o) : common_parent(o)            { set_mixin(o.mixin); }
        basic_sparse_set(basic_sparse_set&& o)      : common_parent(std::move(o)) { set_mixin(std::move(o.mixin)); }
        basic_sparse_set& operator=(const basic_sparse_set& o) { common_parent::operator=(o);            set_mixin(o.mixin);            return *this; }
        basic_sparse_set& operator=(basic_sparse_set&& o)      { common_parent::operator=(std::move(o)); set_mixin(std::move(o.mixin)); return *this; }


        VE_CONTAINER_BEGIN_END((this, dense.begin()), (this, dense.end()));


        /**
         * Inserts the given entity into the set, if no entity with the same index already exists.
         * Note: no *_exact and *_any_version overloads of this method are provided, as their behaviour would be somewhat confusing.
         * To overwrite entities with different versions / unassigned bits, simply call the appropriate erase_* overload first.
         * @param entt The entity to insert.
         * @return A pair [iterator, success], containing an iterator to the inserted entity and a boolean indication whether or not insertion happened.
         */
        std::pair<iterator, bool> insert(entity_type entt) { return insert_impl<versionless_equality_mask>(entt); }


        /** Erases the entity with the same index and version as the provided entity from the set, if one exists. Returns whether erasure happened. */
        bool erase(entity_type entt) { return erase_impl<equality_mask>(entt); }
        /** Erases the entity with the same index, version and unassigned bits as the provided entity from the set, if one exists. Returns whether erasure happened. */
        bool erase_exact(entity_type entt) { return erase_impl<exact_equality_mask>(entt); }
        /** Erases the entity with the same index as the provided entity from the set, if one exists. Returns whether erasure happened. */
        bool erase_any_version(entity_type entt) { return erase_impl<versionless_equality_mask>(entt); }


        /** Returns an iterator to the entity with the same index and version as the provided entity, or end() if no such entity exists. */
        [[nodiscard]] iterator find(entity_type entt) { return find_impl<equality_mask>(this, entt); }
        /** Returns an iterator to the entity with the same index, version and unassigned bits as the provided entity, or end() if no such entity exists. */
        [[nodiscard]] iterator find_exact(entity_type entt) { return find_impl<exact_equality_mask>(this, entt); }
        /** Returns an iterator to the entity with the same index as the provided entity, or end() if no such entity exists. */
        [[nodiscard]] iterator find_any_version(entity_type entt) { return find_impl<versionless_equality_mask>(this, entt); }


        // Const versions.
        using common_parent::find;
        using common_parent::find_exact;
        using common_parent::find_any_version;


        /** Returns the entity with the given index and version. This method <b>DOES NOT</b> validate the entity is within the set. */
        [[nodiscard]] reference operator[](entity_type entt) { return dense[sparse[entity_utils::index_of(entt)]]; }

        // Const version.
        using common_parent::operator[];


        /** Removes all elements from the set. */
        void clear(void) {
            dense.clear();
            sparse.clear();
            if constexpr (has_tombstones) tombstones.clear();

            mixin.on_clear();
        }


        void set_mixin(mixin_type mixin) {
            this->mixin = std::move(mixin);
            this->mixin.on_added_to_set(*this);
        }


        VE_GET_MREFS(mixin);
    private:
        template <typename P> friend class sparse_set_iterator;
        friend common_parent;


        VE_NO_UNIQUE_ADDRESS mixin_type mixin;


        /**
         * Inserts into the set if there is no entity which compares equal according to the provided mask.
         * Returns an iterator to either the existing or inserted element and whether insertion happened.
         */
        template <entity_type Mask> std::pair<iterator, bool> insert_impl(entity_type entt) {
            if (auto it = find_impl<Mask>(this, entt); it != end()) return { it, false };


            std::size_t dense_index;

            if constexpr (has_tombstones) {
                if (!tombstones.empty()) {
                    dense_index = take_back(tombstones);
                    dense[dense_index] = entt;

                    goto insert_sparse;
                }
            }

            dense_index = dense.size();
            dense.push_back(entt);


            insert_sparse:
            const auto sparse_index = entity_utils::index_of(entt);

            if (!sparse.has_page_for(sparse_index) || sparse[sparse_index] == entity_utils::tombstone()) sparse.emplace(sparse_index);
            sparse[sparse_index] = dense_index;


            mixin.on_insert(entt, dense_index);
            return { make_iterator(dense_index), true };
        }


        /**
         * Erases from the set if there is an entity which compares equal according to the provided mask.
         * Returns true if an erasure happened.
         */
        template <entity_type Mask> bool erase_impl(entity_type entt) {
            if (auto it = find_impl<Mask>(this, entt); it != end()) {
                const auto sparse_index = entity_utils::index_of(entt);
                const auto dense_index  = it.dense_offset();


                // Don't need to check for tombstones since swap and pop is used when:
                // - has_tombstones is false, so we don't have to check if the last element is a tombstone.
                // - this is the last element, so we are swapping with ourselves, so the element is not a tombstone.
                auto swap_and_pop = [&] {
                    const auto other_sparse_index = entity_utils::index_of(dense.back());

                    // Note: this is not optimal but provides a cleaner sequence of events from the perspective of the mixin.
                    std::swap(dense[dense_index], dense.back());
                    std::swap(sparse[sparse_index], sparse[other_sparse_index]);
                    mixin.on_swap(dense.back(), dense[dense_index], dense.size() - 1, dense_index);

                    dense.pop_back();
                    sparse.erase(sparse_index);
                    mixin.on_erase(entt, dense.size());
                };


                if constexpr (has_tombstones) {
                    if (dense_index != dense.size() - 1) {
                        tombstones.push_back(dense_index);

                        dense[dense_index] = entity_utils::tombstone();
                        sparse.erase(sparse_index);

                        mixin.on_erase(entt, dense_index);
                    } else swap_and_pop();
                } else swap_and_pop();


                return true;
            }


            return false;
        }


        /** Returns an iterator to the element with the given index in the dense storage. */
        [[nodiscard]] iterator       make_iterator(std::size_t dense_index)       { return iterator       { this, dense.begin() + dense_index }; }
        /** @copydoc make_iterator */
        [[nodiscard]] const_iterator make_iterator(std::size_t dense_index) const { return const_iterator { this, dense.begin() + dense_index }; }
    };




    /** @ref basic_sparse_set with default entity traits and no mixin. */
    using sparse_set = basic_sparse_set<default_entity_traits<VE_DEFAULT_ENTITY_TYPE>>;
}