#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/clientserver/instance_events.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>
#include <VoxelEngine/utility/arbitrary_storage.hpp>

#include <entt/entt.hpp>


namespace ve {
    using instance_dispatcher_t = simple_event_dispatcher<false>;
    struct overridable_function_tag {};


    class instance;


    class instance_registry {
    public:
        static instance_registry& instance(void);


        void update_all(nanoseconds dt);


        void add_instance(class instance* instance) {
            instances.push_back(instance);
        }

        void remove_instance(class instance* instance) {
            std::erase(instances, instance);
        }

    private:
        // Assume few instances.
        std::vector<class instance*> instances;
    };


    // Base class for client & server.
    // Can also be used as unified client / server instance for scenarios where multiplayer support is not required.
    class instance :
        public registry,
        public arbitrary_storage,
        public subscribe_only_view<instance_dispatcher_t>
    {
    public:
        instance(void) : id(random_uuid()) {
            instance_registry::instance().add_instance(this);
        }

        virtual ~instance(void) {
            instance_registry::instance().remove_instance(this);
        }

        ve_immovable(instance);


        void update(nanoseconds dt) {
            VE_PROFILE_FN();

            dispatch_event(instance_pre_tick_event { dt, tick_count });

            registry::update(dt);
            update(dt, overridable_function_tag { });

            ++tick_count;
            dispatch_event(instance_post_tick_event { dt, tick_count });
        }


        virtual std::string get_name(void) const {
            return "unified instance "s + boost::uuids::to_string(id);
        }


        VE_GET_CREF(id);
        VE_GET_VAL(tick_count);
    protected:
        // Prevent overriding the main update method so we can force the dispatching of the events to occur
        // before and after all other update calls.
        virtual void update(nanoseconds dt, overridable_function_tag) {}

    private:
        instance_id id;
        u64 tick_count = 0;
    };
}