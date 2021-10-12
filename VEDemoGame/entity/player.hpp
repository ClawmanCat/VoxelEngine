#pragma once

#include <VEDemoGame/game.hpp>

#include <VoxelEngine/ecs/ecs.hpp>


namespace demo_game {
    class player : public ve::static_entity {
    public:
        using camera_t = ve::gfx::perspective_camera;


        explicit player(ve::registry& registry, camera_t* camera) : ve::static_entity(registry) {
            camera_controller.camera = camera;
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::camera_controller_component<camera_t> VE_COMPONENT(camera_controller) = ve::camera_controller_component<camera_t> { };
    };
}