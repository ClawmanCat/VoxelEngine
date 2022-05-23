#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    const inline std::vector<GLuint> muted_messages {
        131222,
        131186,
        131154
    };


    inline void GLAPIENTRY opengl_logging_callback(
        GLenum source,
        GLenum type,
        GLuint message_id,
        GLenum severity,
        GLsizei message_length,
        const GLchar* message,
        const void* data
    ) {
        if (ranges::contains(muted_messages, message_id)) return;


        #define ve_impl_gl_msg_source(source) case GL_DEBUG_SOURCE_##source: source_string = #source; break;

        std::string_view source_string;
        switch (source) {
            ve_impl_gl_msg_source(API);
            ve_impl_gl_msg_source(WINDOW_SYSTEM);
            ve_impl_gl_msg_source(SHADER_COMPILER);
            ve_impl_gl_msg_source(THIRD_PARTY);
            ve_impl_gl_msg_source(APPLICATION);
            ve_impl_gl_msg_source(OTHER);
        }


        #define ve_impl_gl_msg_type(type) case GL_DEBUG_TYPE_##type: type_string = #type; break;

        std::string_view type_string;
        switch (type) {
            ve_impl_gl_msg_type(ERROR);
            ve_impl_gl_msg_type(DEPRECATED_BEHAVIOR);
            ve_impl_gl_msg_type(UNDEFINED_BEHAVIOR);
            ve_impl_gl_msg_type(PORTABILITY);
            ve_impl_gl_msg_type(PERFORMANCE);
            ve_impl_gl_msg_type(MARKER);
            ve_impl_gl_msg_type(PUSH_GROUP);
            ve_impl_gl_msg_type(POP_GROUP);
            ve_impl_gl_msg_type(OTHER);
        }


        #define ve_impl_gl_msg_severity(severity, log_level) case GL_DEBUG_SEVERITY_##severity: level = logger::level::log_level; break;

        logger::level level;
        switch (severity) {
            ve_impl_gl_msg_severity(NOTIFICATION, DEBUG);
            ve_impl_gl_msg_severity(LOW,          INFO);
            ve_impl_gl_msg_severity(MEDIUM,       INFO);
            ve_impl_gl_msg_severity(HIGH,         WARNING);
        }


        loggers::get_gfxapi_logger().message(
            cat(source_string, "/", type_string, ": ", std::string { message, message + message_length }, " (", message_id, ")"),
            level
        );


        VE_DEBUG_ONLY(
            if (level >= logger::level::WARNING) VE_BREAKPOINT;
        );
    }


    inline void register_opengl_logger(decltype(&opengl_logging_callback) fn = opengl_logging_callback) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(fn, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
}