#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/utility.hpp>

#include <GL/glew.h>

#include <string_view>


#define VE_IMPL_GL_SWITCH_CASE(out, entry) \
case entry: out = #entry; break;


namespace ve::graphics {
    static void GLAPIENTRY opengl_error_callback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* params
    ) {
        std::string_view source_str;
        switch (source) {
            VE_IMPL_GL_SWITCH_CASE(source_str, GL_DEBUG_SOURCE_API)
            VE_IMPL_GL_SWITCH_CASE(source_str, GL_DEBUG_SOURCE_WINDOW_SYSTEM)
            VE_IMPL_GL_SWITCH_CASE(source_str, GL_DEBUG_SOURCE_SHADER_COMPILER)
            VE_IMPL_GL_SWITCH_CASE(source_str, GL_DEBUG_SOURCE_THIRD_PARTY)
            VE_IMPL_GL_SWITCH_CASE(source_str, GL_DEBUG_SOURCE_APPLICATION)
            VE_IMPL_GL_SWITCH_CASE(source_str, GL_DEBUG_SOURCE_OTHER)
            default: break;
        }
        
        source_str.remove_prefix("GL_DEBUG_SOURCE_"sv.length());
        
        
        std::string_view type_str;
        switch (type) {
            VE_IMPL_GL_SWITCH_CASE(type_str, GL_DEBUG_TYPE_ERROR)
            VE_IMPL_GL_SWITCH_CASE(type_str, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
            VE_IMPL_GL_SWITCH_CASE(type_str, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
            VE_IMPL_GL_SWITCH_CASE(type_str, GL_DEBUG_TYPE_PORTABILITY)
            VE_IMPL_GL_SWITCH_CASE(type_str, GL_DEBUG_TYPE_PERFORMANCE)
            VE_IMPL_GL_SWITCH_CASE(type_str, GL_DEBUG_TYPE_MARKER)
            VE_IMPL_GL_SWITCH_CASE(type_str, GL_DEBUG_TYPE_PUSH_GROUP)
            VE_IMPL_GL_SWITCH_CASE(type_str, GL_DEBUG_TYPE_POP_GROUP)
            VE_IMPL_GL_SWITCH_CASE(type_str, GL_DEBUG_TYPE_OTHER)
            default: break;
        }
        
        type_str.remove_prefix("GL_DEBUG_TYPE_"sv.length());
        
        
        std::string_view severity_str;
        switch (severity) {
            VE_IMPL_GL_SWITCH_CASE(severity_str, GL_DEBUG_SEVERITY_HIGH)
            VE_IMPL_GL_SWITCH_CASE(severity_str, GL_DEBUG_SEVERITY_MEDIUM)
            VE_IMPL_GL_SWITCH_CASE(severity_str, GL_DEBUG_SEVERITY_LOW)
            VE_IMPL_GL_SWITCH_CASE(severity_str, GL_DEBUG_SEVERITY_NOTIFICATION)
            default: break;
        }
        
        severity_str.remove_prefix("GL_DEBUG_SEVERITY_"sv.length());
        
        
        logger::level level = logger::level::DEBUG;
        
        // OpenGL constants don't match their supposed type (GLenum). Just cast the first argument to prevent warnings.
        using real_enum_t = decltype(GL_DEBUG_TYPE_ERROR);
        
        if (one_of((real_enum_t) type, GL_DEBUG_TYPE_ERROR, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)) level = logger::level::ERROR;
        if (one_of((real_enum_t) type, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DEBUG_TYPE_PERFORMANCE, GL_DEBUG_TYPE_PORTABILITY)) level = logger::level::WARNING;
        
        loggers::ve_logger.message(
            "OpenGL "s + type_str + " event with severity " + severity_str + " caught from " + source_str + ": " + message,
            level
        );
        
        
        // Put breakpoint here to debug errors.
        if (level == logger::level::ERROR) {
            ((void) 0);
        }
    }
}