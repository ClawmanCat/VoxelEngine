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
    class system_remote_init : public system<system_remote_init, meta::pack<remote_init_component>> {
    public:
        explicit system_remote_init(u16 priority = priority::HIGHEST) : priority(priority) {}


        u16 get_priority(void) const {
            return priority;
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            for (auto entity : view) {
                const auto& cmp = view.template get<remote_init_component>(entity);

                if (auto it = initializers.find(cmp.type); it != initializers.end()) {
                    it->second->init(entity, owner, cmp);
                }
            }

            // Component will not get synchronized again unless changed.
            owner.template remove_all_components<remote_init_component>();
        }


        template <typename Pred, typename T = std::remove_cvref_t<typename meta::function_traits<Pred>::arguments::template get<2>>>
        requires std::is_invocable_v<Pred, entt::entity, ve::registry&, const T&>
        void add_initializer(Pred&& pred) {
            initializers.emplace(
                type_hash<T>(),
                make_unique<initializer_data<Pred, T>>(fwd(pred))
            );
        }
    private:
        struct initializer_data_base {
            virtual ~initializer_data_base(void) = default;
            virtual void init(entt::entity entity, registry& owner, const remote_init_component& cmp) = 0;
        };

        template <typename Pred, typename T> struct initializer_data : public initializer_data_base {
            Pred pred;

            explicit initializer_data(Pred pred) : initializer_data_base(), pred(std::move(pred)) {}

            void init(entt::entity entity, registry& owner, const remote_init_component& cmp) override {
                std::invoke(pred, entity, owner, serialize::from_bytes<T>(cmp.data));
            }
        };


        hash_map<u64, unique<initializer_data_base>> initializers;
        u16 priority;
    };
}