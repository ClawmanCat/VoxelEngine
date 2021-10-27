#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/graphics/presentation/present_mode.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/api_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/presentation/window_helpers.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/target.hpp>


namespace ve::gfx {
    class window;
}


namespace ve::gfx::opengl {
    class canvas : public render_target {
    public:
        canvas(window* owner, present_mode_t mode);


        void bind(void) override;
        void begin_frame(void);
        void end_frame(void);


        void set_present_mode(present_mode_t mode);

        VE_GET_VAL(owner);
        VE_GET_VAL(mode);
    private:
        window* owner;
        present_mode_t mode;
        bool warned_on_closed_render = false;

        void make_current(void);
    };
}