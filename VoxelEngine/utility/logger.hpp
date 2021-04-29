#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/streamable.hpp>

#include <string>
#include <string_view>
#include <ostream>
#include <mutex>
#include <utility>


#ifdef VE_DEBUG
    #define VE_LOG_DEBUG(...) ve::loggers::ve_logger.debug(__VA_ARGS__)
#else
    #define VE_LOG_DEBUG(...) /* Not Enabled. */
#endif

#define VE_LOG_INFO(...)  ve::loggers::ve_logger.info (__VA_ARGS__)
#define VE_LOG_WARN(...)  ve::loggers::ve_logger.warn (__VA_ARGS__)
#define VE_LOG_ERROR(...) ve::loggers::ve_logger.error(__VA_ARGS__)
#define VE_LOG_FATAL(...) ve::loggers::ve_logger.fatal(__VA_ARGS__)


namespace ve {
    class logger {
    public:
        enum class level { DEBUG, INFO, WARNING, ERROR, FATAL };
        
        logger(universal<std::string> auto&& name, level logger_level, meta::streamable auto&... targets) :
            name(std::forward<std::string>(name)),
            logger_level(logger_level),
            targets{ std::ref(targets)... }
        {}
    
        void message(std::string_view msg, level msg_level = level::INFO, bool show_info = true);
        
        
        // Note: the below functions are not disabled in release builds automatically.
        // If this behavior is required, use the provided VE_LOG_* macros.
        void debug(std::string_view msg, bool show_info = true) { message(msg, level::DEBUG,   show_info); }
        void info (std::string_view msg, bool show_info = true) { message(msg, level::INFO,    show_info); }
        void warn (std::string_view msg, bool show_info = true) { message(msg, level::WARNING, show_info); }
        void error(std::string_view msg, bool show_info = true) { message(msg, level::ERROR,   show_info); }
        void fatal(std::string_view msg, bool show_info = true) { message(msg, level::FATAL,   show_info); }
    private:
        std::string name;
        level logger_level;
        
        small_vector<ref<std::ostream>> targets;
        std::mutex targets_mtx;
        
        
        static std::string make_time_string(const char* fmt = "%a, %F %T");
    };
    
    
    namespace loggers {
        namespace detail {
            extern logger& get_ve_logger(void);
        }
        
        // Default logger used by the engine.
        inline logger& ve_logger = detail::get_ve_logger();
    }
}