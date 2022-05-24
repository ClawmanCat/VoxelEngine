#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/view.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    // The remote initializer system invokes a user-defined initializer method whenever a specific component is added to an entity.
    // This can be useful to perform initialization of entities synchronized from a remote instance.
    template <template <typename System> typename... Mixins>
    class system_remote_initializer
        : public system<system_remote_initializer<Mixins...>, create_empty_view, meta::pack<>, deduce_component_access, Mixins...>
    {
    public:
        explicit system_remote_initializer(u16 priority = priority::NORMAL) : priority(priority) {}


        u16 get_priority(void) const {
            return priority;
        }


        void on_system_added  (registry& owner) { this->owner = &owner;  }
        void on_system_removed(registry& owner) { this->owner = nullptr; }


        void on_system_update(registry& owner, view_type view, nanoseconds dt) {
            for (const auto& s : subsystems) s->update(owner);
        }


        template <
            meta::pack_of_types RequiredTags = meta::pack<>,
            meta::pack_of_types ExcludedTags = meta::pack<>,
            typename Initializer,
            typename Component = meta::nth_argument_base<Initializer, 2>
        > requires (
            std::is_invocable_v<Initializer, registry&, entt::entity, const Component&>
        ) void add_initializer(Initializer init) {
            subsystems.push_back(
                make_unique<subsystem<Initializer, Component, RequiredTags, ExcludedTags>>(this, std::move(init))
            );
        }
    private:
        struct subsystem_base {
            virtual ~subsystem_base(void) = default;
            virtual void update(registry& owner) = 0;
        };


        template <typename Initializer, typename Component, meta::pack_of_types RequiredTags, meta::pack_of_types ExcludedTags>
        struct subsystem : public subsystem_base {
            Initializer init;
            system_remote_initializer* parent;
            event_handler_id_t handler;
            entt::sparse_set newly_added;


            explicit subsystem(system_remote_initializer* parent, Initializer&& init) : init(fwd(init)), parent(parent) {
                handler = parent->owner->template add_raw_handler([this] (const component_created_event<Component>& e) {
                    if (!newly_added.contains(e.entity)) newly_added.emplace(e.entity);
                });
            }

            ~subsystem(void) {
                parent->owner->template remove_handler<component_created_event<Component>>(handler);
            }

            ve_immovable(subsystem);


            void update(registry& owner) override {
                auto view = view_from_set(newly_added) | owner.template view_pack<
                    typename RequiredTags::template append<Component>,
                    ExcludedTags
                >();

                for (auto entity : view) {
                    std::invoke(init, owner, entity, view.template get<Component>(entity));
                }

                newly_added.clear();
            }
        };


        template <typename Initializer, typename Component, meta::pack_of_types RequiredTags, meta::pack_of_types ExcludedTags>
        friend struct subsystem;


        u16 priority;
        registry* owner;
        std::vector<unique<subsystem_base>> subsystems;
    };
}