#pragma once

#include <VoxelEngine/core/core.hpp>

#include <VoxelEngine/ecs/entt_include.hpp>


namespace ve {
    class registry;
}


namespace ve::detail {
    // registry is incomplete here, this must be handled in the cpp file.
    extern entt::registry& get_storage(registry& registry);


    // Storage for system data.
    struct system_data_base {
        explicit system_data_base(u16 priority) : priority(priority) {}

        virtual ~system_data_base(void) = default;
        virtual void update(registry& self, nanoseconds dt) = 0;
        virtual void init(registry& self) = 0;
        virtual void uninit(registry& self) = 0;

        u16 priority;
    };

    template <typename System> struct system_data : system_data_base {
        system_data(System&& system, u16 priority) : system_data_base(priority), system(std::move(system)) {}

        void update(registry& self, nanoseconds dt) override {
            system.update(self, System::make_view(get_storage(self)), dt);
        }

        void init(registry& self) override {
            system.init(self);
        }

        void uninit(registry& self) override {
            system.uninit(self);
        }

        System system;
    };


    // Storage for static entities.
    // Wrapper is required since static_entity does not provide a virtual destructor.
    struct static_entity_storage_base {
        virtual ~static_entity_storage_base(void) = default;
    };

    template <typename Entity> struct static_entity_storage : static_entity_storage_base {
        explicit static_entity_storage(Entity&& entity) : entity(std::move(entity)) {}

        Entity entity;
    };
}