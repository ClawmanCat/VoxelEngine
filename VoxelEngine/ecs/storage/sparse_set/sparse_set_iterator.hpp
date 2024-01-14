#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_mixin.hpp>
#include <VoxelEngine/utility/container/container_typedefs.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/utility/meta/conditional_evaluation.hpp>


namespace ve::ecs {
    /**
     * Iterator type for basic_sparse_set. Skips over any tombstones contained within the sparse set, if they exist.
     * Iterator is invalidated if an element is added or removed from the sparse set.
     * @tparam Parent The sparse set type this iterator is associated with.
     */
    template <typename Parent> class sparse_set_iterator {
    public:
        VE_WRAP_TYPEDEFS(Parent, entity_traits, entity_type, entity_utils);
        VE_WRAP_CONSTANTS(Parent, has_index_stability, has_tombstones);
        VE_CONTAINER_TYPEDEFS(entity_type);

        using self_type            = sparse_set_iterator<Parent>;
        using sparse_set_type      = Parent;
        using nested_iterator_type = std::conditional_t<std::is_const_v<Parent>, typename Parent::dense_type::const_iterator, typename Parent::dense_type::iterator>;
        using iterator_category    = std::bidirectional_iterator_tag;
        using mixin_type           = VE_EVAL_TYPE_IF_VALID(meta::null_type, typename Parent::mixin_type);

        constexpr static inline bool has_mixin         = VE_EVAL_IF_VALID(false, Parent::has_mixin);
        constexpr static inline bool is_const_iterator = std::is_const_v<Parent>;


        sparse_set_iterator(void) : parent(nullptr), iterator() {}
        sparse_set_iterator(sparse_set_type* parent, nested_iterator_type iterator) : parent(parent), iterator(iterator) { seek_until_valid(); }


        /** Copy / move operations: allow conversion from iterator to const_iterator. */
        template <iterator_compatible_parent<Parent> OP> sparse_set_iterator(const sparse_set_iterator<OP>& other) { *this = other; }
        template <iterator_compatible_parent<Parent> OP> sparse_set_iterator(sparse_set_iterator<OP>&& other) { *this = std::move(other); }

        template <iterator_compatible_parent<Parent> OP> sparse_set_iterator& operator=(const sparse_set_iterator<OP>& other) {
            VE_COPY_FIELDS(other, parent, iterator);
            return *this;
        }

        template <iterator_compatible_parent<Parent> OP> sparse_set_iterator& operator=(sparse_set_iterator<OP>&& other) {
            VE_MOVE_FIELDS(other, parent, iterator);
            return *this;
        }


        /**
         * Retrieve the currently iterated element.
         * Note: returned type is always const since index section of the entity ID is immutable.
         * Use set_version and set_unassigned_bits to edit the remaining section of the entity ID.
         */
        [[nodiscard]] const_reference operator* (void) const { return  *iterator; }
        /** @copydoc operator* */
        [[nodiscard]] const_pointer   operator->(void) const { return &*iterator; }


        self_type& operator++(void) { next(); return *this; }
        self_type& operator--(void) { prev(); return *this; }
        self_type  operator++(int)  { auto old = this; next(); return old; }
        self_type  operator--(int)  { auto old = this; prev(); return old; }


        template <typename OP> [[nodiscard]] auto operator<=>(const sparse_set_iterator<OP>& o) const {
            return iterator <=> o.iterator;
        }

        template <typename OP> [[nodiscard]] bool operator==(const sparse_set_iterator<OP>& o) const {
            return iterator == o.iterator;
        }


        /** Sets the version of the current entity. Provided version will be shifted to the correct bit index before being applied. */
        void set_version(entity_type version) const requires (!is_const_iterator) {
            entity_type prev = *iterator;

            *iterator &= ~entity_utils::version_mask();
            *iterator |= (version << entity_traits::index_bits) & entity_utils::version_mask();

            parent->mixin.on_edited(prev, *iterator, dense_offset());
        }


        /** Sets the unassigned bits of the current entity. Provided bits will be shifted to the correct bit index before being applied. */
        void set_unassigned_bits(entity_type ubits) const requires (!is_const_iterator && entity_utils::unassigned_bits_mask() != 0) {
            entity_type prev = *iterator;

            *iterator &= ~entity_utils::unassigned_bits_mask();
            *iterator |= (ubits << (entity_traits::index_bits + entity_traits::version_bits)) & entity_utils::unassigned_bits_mask();

            parent->mixin.on_edited(prev, *iterator, dense_offset());
        }


        /** Returns the index of the current entity inside the dense vector of its container. */
        [[nodiscard]] std::size_t dense_offset(void) const {
            return std::distance(parent->dense.begin(), iterator);
        }


        VE_GET_VALS(parent);
    private:
        template <typename OP> friend class sparse_set_iterator;


        sparse_set_type* parent;
        nested_iterator_type iterator;


        void prev(void) {
            --iterator;

            if constexpr (has_tombstones) {
                const auto begin = parent->dense.begin();

                // If tombstones can exist keep decrementing until we're on a valid slot or at the beginning of storage.
                while (iterator != begin && *iterator == entity_utils::tombstone()) --iterator;
                // If we're at the beginning of storage, increment to the first valid slot.
                if (iterator == begin && *iterator == entity_utils::tombstone()) [[unlikely]] seek_until_valid();
            }
        }


        void next(void) {
            ++iterator;
            seek_until_valid();
        }


        void seek_until_valid(void) {
            // Increment the iterator until we reach a non-tombstone element or the end of storage.
            if constexpr (has_tombstones) {
                const auto end = parent->dense.end();
                while (iterator != end && *iterator == entity_utils::tombstone()) ++iterator;
            }
        }
    };
}