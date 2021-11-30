#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/remote_init_component.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    // system_remote_init maps type tags to initializer functions.
    // A remote may add a remote_init_component with the same type tag to an entity and synchronize it,
    // causing initialization to occur on the other end, using the initializer mapping to that type tag.
    template <typename Pred = fn<void, entt::entity>> requires (std::is_invocable_v<Pred, entt::entity>)
    class system_remote_init : public system<
        system_remote_init<Pred>,
        meta::pack<remote_init_component>,
        meta::pack<>
    > {
    public:
        explicit system_remote_init(u16 priority = priority::HIGHEST) : priority(priority) {}


        u16 get_priority(void) const {
            return priority;
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            for (auto entity : view) {
                u64 type = view.template get<remote_init_component>(entity).type;

                if (auto it = initializers.find(type); it != initializers.end()) [[likely]] {
                    std::invoke(it->second, entity);
                } else {
                    VE_LOG_ERROR(cat("Remote requested initialization of entity using unknown initializer ", type, "."));
                }
            }

            // Component will not get synchronized again unless changed.
            owner.template remove_all_components<remote_init_component>();
        }


        template <typename Tag> void add_initializer(Pred pred) {
            initializers[type_hash<Tag>()].push_back(std::move(pred));
        }
    private:
        hash_map<u64, small_vector<Pred, 1>> initializers;
        u16 priority;
    };
}