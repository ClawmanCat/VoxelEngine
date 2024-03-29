#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/clientserver/instance_events.hpp>
#include <VoxelEngine/clientserver/message_handler.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>
#include <VoxelEngine/utility/arbitrary_storage.hpp>

#include <VoxelEngine/ecs/entt_include.hpp>


namespace ve {
    class instance;


    class instance_registry {
    public:
        static instance_registry& instance(void);
        ~instance_registry(void);


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
    class instance : public registry, public arbitrary_storage {
    public:
        enum instance_type { CLIENT, SERVER, UNIFIED };


        instance(instance_type type = UNIFIED) : id(random_uuid()), type(type) {
            instance_registry::instance().add_instance(this);
            get_validator().allow_by_default(change_result::ALLOWED);
        }

        virtual ~instance(void) {
            if (registry_alive) instance_registry::instance().remove_instance(this);
        }

        ve_immovable(instance);


        void update(nanoseconds dt) {
            VE_PROFILE_FN();

            last_dt = dt;
            dispatch_event(instance_pre_tick_event { dt, tick_count });

            registry::update(dt);
            update(dt, overridable_function_tag { });

            ++tick_count;
            dispatch_event(instance_post_tick_event { dt, tick_count });
        }


        virtual std::string get_name(void) const {
            return "unified instance "s + boost::uuids::to_string(id);
        }


        virtual std::vector<shared<message_handler>> get_connections(void) {
            return { };
        }


        virtual shared<message_handler> get_connection(instance_id remote) {
            throw std::runtime_error { "Cannot get connection from unified instance." };
        }


        VE_GET_MREF(mtr);
        VE_GET_CREF(id);
        VE_GET_VAL(type);
        VE_GET_VAL(tick_count);
        VE_GET_VAL(last_dt);
    protected:
        // Prevent overriding the main update method so we can force the dispatching of the events to occur
        // before and after all other update calls.
        virtual void update(nanoseconds dt, overridable_function_tag) {}

    private:
        // Instances are typically declared as globals or static class members, so we can't control if it's the instance
        // or the registry that's destroyed first. So just set a flag in the instance if the registry is gone.
        // TODO: Find a cleaner way to do this.
        friend class instance_registry;
        bool registry_alive = true;

        message_type_registry mtr;
        instance_id id;
        instance_type type;

        u64 tick_count = 0;
        nanoseconds last_dt = 1ns;
    };
}