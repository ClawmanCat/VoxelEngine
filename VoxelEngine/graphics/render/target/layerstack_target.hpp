#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/render/target/target.hpp>

#include <string>


namespace ve {
    class layerstack_target : public render_target {
    public:
        layerstack_target(void) = default;
        
        template <universal<std::string> String>
        layerstack_target(String&& name, shared<graphics_pipeline>&& provider) :
            render_target(std::move(provider)),
            name(std::forward<String>(name))
        {}
        
        layerstack_target(layerstack_target&&) = default;
        layerstack_target& operator=(layerstack_target&&) = default;
        
        
        [[nodiscard]] std::string_view get_name(void) const noexcept { return name; }
    private:
        std::string name;
    };
}