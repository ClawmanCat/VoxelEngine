#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/functional.hpp>


namespace ve::gfx::opengl {
    template <typename Resource, typename Destructor = std::function<void(Resource&&)>>
    struct gl_resource {
        Resource resource = nullptr;
        Destructor destructor;


        explicit gl_resource(Resource&& resource, Destructor&& destructor = no_op)
            : resource(std::move(resource)), destructor(std::move(destructor))
        {}

        ~gl_resource(void) {
            if (resource) destructor(std::move(resource));
        }


        ve_swap_move_only(gl_resource, resource, destructor);
        ve_dereference_as(resource);

        constexpr operator Resource&(void) { return resource; }
        constexpr operator const Resource&(void) const { return resource; }


        constexpr Resource release(void) {
            return std::exchange(resource, nullptr);
        }
    };
}