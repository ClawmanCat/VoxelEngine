#pragma once

#include <VoxelEngine/core/core.hpp>


#ifndef VE_GRAPHICS_API
    #error Please define a graphics api to use. (#define VE_GRAPHICS_API opengl or #define VE_GRAPHICS_API vulkan)
#endif


#define graphics_include(header) <VoxelEngine/platform/graphics/VE_GRAPHICS_API/header>


// Include all graphics headers containing parts of the public API.
#include graphics_include(buffer/attribute.hpp)
#include graphics_include(buffer/vertex.hpp)
#include graphics_include(buffer/buffer.hpp)
#include graphics_include(buffer/vertex_buffer.hpp)
#include graphics_include(buffer/indexed_vertex_buffer.hpp)

#include graphics_include(pipeline/pipeline.hpp)

#include graphics_include(shader/shader_program.hpp)
#include graphics_include(shader/shader_library.hpp)
#include graphics_include(shader/shader_layout.hpp)

#include graphics_include(target/target.hpp)
#include graphics_include(target/layerstack_target.hpp)
#include graphics_include(target/framebuffer_target.hpp)

#include graphics_include(texture/format.hpp)
#include graphics_include(texture/texture.hpp)
#include graphics_include(texture/texture_loader.hpp)
#include graphics_include(texture/texture_atlas.hpp)
#include graphics_include(texture/aligned_texture_atlas.hpp)
#include graphics_include(texture/generative_texture_atlas.hpp)
#include graphics_include(texture/texture_manager.hpp)

#include graphics_include(uniform/uniform_storage.hpp)

#include graphics_include(window/window.hpp)
#include graphics_include(window/layerstack.hpp)

#include graphics_include(context.hpp)