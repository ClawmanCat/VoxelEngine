#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/priority.hpp>
#include <VoxelEngine/utility/arbitrary_storage.hpp>


namespace ve::gfx {
    class shader_preprocessor {
    public:
        explicit shader_preprocessor(std::string name, u16 priority = priority::NORMAL) : priority(priority), name(std::move(name)) {}
        virtual ~shader_preprocessor(void) = default;

        // Performs some transform on the shader source code.
        // Context can be used to pass data between preprocessor stages.
        virtual void operator()(std::string& src, arbitrary_storage& context) const = 0;

        VE_GET_VAL(priority);
        VE_GET_CREF(name);
    private:
        u16 priority;
        std::string name;
    };
}