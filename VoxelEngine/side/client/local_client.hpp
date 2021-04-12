#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/client/client.hpp>
#include <VoxelEngine/ecs/scene.hpp>


namespace ve {
    class local_client : public client {
    public:
        VE_GET_MREF(scene);
    private:
        scene<side::CLIENT> scene;
    };
}