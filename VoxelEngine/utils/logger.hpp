#pragma once

#include <VoxelEngine/core/core.hpp>

#include <string>
#include <string_view>
#include <ostream>
#include <mutex>


#ifdef VE_DEBUG
    #define VE_LOG_DEBUG(...) ve::loggers::ve_logger().debug(__VA_ARGS__)
    #define VE_LOG_INFO(...)  ve::loggers::ve_logger().info (__VA_ARGS__)
    #define VE_LOG_WARN(...)  ve::loggers::ve_logger().warn (__VA_ARGS__)
    #define VE_LOG_ERROR(...) ve::loggers::ve_logger().error(__VA_ARGS__)
    #define VE_LOG_FATAL(...) ve::loggers::ve_logger().fatal(__VA_ARGS__)
#else
    #define VE_LOG_DEBUG(...) /* Not Enabled. */
    #define VE_LOG_INFO(...)  /* Not Enabled. */
    #define VE_LOG_WARN(...)  ve::loggers::ve_logger().warn (__VA_ARGS__)
    #define VE_LOG_ERROR(...) ve::loggers::ve_logger().error(__VA_ARGS__)
    #define VE_LOG_FATAL(...) ve::loggers::ve_logger().fatal(__VA_ARGS__)
#endif


namespace ve {
    class logger {
    public:
        enum class level {
            DEBUG, INFO, WARNING, ERROR, FATAL
        };
        
        
        logger(std::string&& name, level logger_level, std::ostream& target);
        
        
        void message(std::string_view msg, level msg_level, bool show_info = true);
        
        void debug(std::string_view msg, bool show_info = true) { message(msg, level::DEBUG,   show_info); }
        void info (std::string_view msg, bool show_info = true) { message(msg, level::INFO,    show_info); }
        void warn (std::string_view msg, bool show_info = true) { message(msg, level::WARNING, show_info); }
        void error(std::string_view msg, bool show_info = true) { message(msg, level::ERROR,   show_info); }
        void fatal(std::string_view msg, bool show_info = true) { message(msg, level::FATAL,   show_info); }
    private:
        std::string name;
        level logger_level;
        std::ostream& target;
        std::mutex target_mtx;
        
        
        static std::string make_time_string(const char* fmt = "%a, %F %T");
    };
    
    
    namespace loggers {
        extern logger& ve_logger(void) noexcept;
    }
}