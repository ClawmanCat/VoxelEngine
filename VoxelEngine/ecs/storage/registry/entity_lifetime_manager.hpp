#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_mixin.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>


namespace ve::ecs {
    /**
     * Class for managing the creation and destruction of entities.
     * @tparam Traits Entity traits for the entities managed by this class.
     * @tparam EMixin Optional mixin passed to the set of entities.
     * @tparam TMixin Optional mixin passed to the set of tombstones.
     */
    template <
        entity_traits Traits,
        sparse_set_mixin_type<Traits> EMixin = no_sparse_set_mixin<Traits>,
        sparse_set_mixin_type<Traits> TMixin = no_sparse_set_mixin<Traits>
    > class entity_lifetime_manager {
    public:
        using entity_type   = typename Traits::type;
        using entity_traits = Traits;
        using entity_utils  = entity_utils<Traits>;


        entity_lifetime_manager(void) = default;

        explicit entity_lifetime_manager(EMixin e_mixin, TMixin t_mixin = TMixin { }) :
            entities(std::move(e_mixin)),
            tombstones(std::move(t_mixin))
        {}


        /**
         * Creates a new entity.
         * @return The ID of the created entity.
         */
        [[nodiscard]] entity_type create(void) {
            entity_type id = tombstones.empty()
                ? next()
                : take_at(tombstones, *std::prev(tombstones.end()));

            auto [it, success] = entities.insert(id);
            VE_DEBUG_ASSERT(success, "Failed to create entity.");

            return id;
        }


        /**
         * Creates an entity with the given ID, if no entity with the same index already exists and no entity with the same ID has existed in the past.
         * @param entt The entity ID to create.
         * @return True if the entity could be created, false otherwise.
         */
        bool create_with_id(entity_type entt) {
            // Do not create the entity if it already exists with a possibly-different version.
            if (entities.contains_any_version(entt)) {
                return false;
            }

            // If the entity existed in the past, the new version must be bigger than its last version.
            // We also have to remove the tombstone if the new version does turn out to be valid.
            if (auto it = tombstones.find_any_version(entt); it != tombstones.end()) {
                if (entity_utils::version_of(*it) <= entity_utils::version_of(entt)) {
                    tombstones.erase(*it);
                } else return false;
            }

            auto [it, success] = entities.insert(entt);
            VE_DEBUG_ASSERT(success, "Failed to create entity with ID {}.", entt);

            return true;
        }


        /** Destroys the given entity, if it exists. Returns whether or not the entity existed. */
        bool destroy(entity_type entt) {
            bool success = entities.erase(entt);

            // Do not store unassigned bits in tombstone values.
            tombstones.insert(entity_utils::next_version(entt) & ~entity_utils::unassigned_bits_mask());

            return success;
        }


        /** Returns true if this entity has existed in the past or still exists currently. */
        [[nodiscard]] bool has_existed(entity_type entt) const {
            return is_alive() || is_dead();
        }


        /** Returns true if this entity exists currently. */
        [[nodiscard]] bool is_alive(entity_type entt) const {
            return entities.contains(entt);
        }


        /** Returns true if this entity has existed in the past but does not exist anymore. */
        [[nodiscard]] bool is_dead(entity_type entt) const {
            if (auto it = tombstones.find_any_version(entt); it != tombstones.end()) {
                return entity_utils::version_of(*it) > entity_utils::version_of(entt);
            }

            if (auto it = entities.find_any_version(entt); it != entities.end()) {
                return entity_utils::version_of(*it) > entity_utils::version_of(entt);
            }

            return false;
        }


        VE_GET_CREFS(entities, tombstones);
    private:
        basic_sparse_set<entity_traits, EMixin> entities;
        basic_sparse_set<entity_traits, TMixin> tombstones;
        entity_type next_index = 0;


        [[nodiscard]] entity_type next(void) {
            // While it would be possible to just check if our new ID matches any tombstones,
            // we don't call next() if there are tombstones to use instead anyway, so it's faster to just skip checking the tombstones altogether.
            // This assert is here in case this behaviour changes in the future.
            VE_DEBUG_ASSERT(tombstones.empty(), "Cannot generate new entity ID while tombstones exist.");

            while (entities.contains_any_version(next_index)) {
                next_index = entity_utils::next_index(next_index);
            }

            return next_index;
        }
    };
}