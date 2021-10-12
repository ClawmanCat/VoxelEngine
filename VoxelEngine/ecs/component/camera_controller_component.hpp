#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // Given an entity with a transform component and a camera controller component,
    // the provided camera is synchronized to that entity's transform.
    template <typename Camera> requires requires { typename Camera::camera_tag; }
    struct camera_controller_component {
        Camera* camera;
    };
}