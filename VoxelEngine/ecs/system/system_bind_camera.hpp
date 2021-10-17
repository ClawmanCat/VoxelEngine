#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>
#include <VoxelEngine/ecs/component/camera_controller_component.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    template <typename Camera> class system_bind_camera : public system<
        system_bind_camera<Camera>,
        meta::pack<transform_component, camera_controller_component<Camera>>
    > {
    public:
        explicit system_bind_camera(u16 priority = priority::NORMAL) : priority(priority) {}


        u16 get_priority(void) const {
            return priority;
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            for (auto entity : view) {
                const auto& [transform, camera] = view.template get<
                    transform_component,
                    camera_controller_component<Camera>
                >(entity);

                camera.camera->set_position(transform.position);
                camera.camera->set_rotation(transform.rotation);
            }
        }

    private:
        u16 priority;
    };
}