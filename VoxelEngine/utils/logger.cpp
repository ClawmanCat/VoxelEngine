#include <VoxelEngine/utils/logger.hpp>
#include <VoxelEngine/utils/io/paths.hpp>
#include <VoxelEngine/utils/io/mkdir_ofstream.hpp>
#include <VoxelEngine/utils/tee_stream.hpp>

#include <magic_enum.hpp>

#include <sstream>
#include <fstream>
#include <utility>
#include <ctime>
#include <iomanip>
#include <iostream>


namespace ve {
    logger::logger(std::string&& name, level logger_level, std::ostream& target) :
        name(std::move(name)),
        logger_level(logger_level),
        target(target)
    { }
    
    
    void logger::message(std::string_view msg, level msg_level, bool show_info) {
        std::stringstream out_message;
        
        if (show_info) {
            out_message << logger::make_time_string() << " ";
            out_message << '[' << name << ", " << magic_enum::enum_name(msg_level) << "] ";
        }
        
        out_message << msg << '\n';
        
        {
            // TODO: Use std::osyncstream once Clang implements it.
            std::lock_guard lock { target_mtx };
            target << out_message.str();

            #ifdef VE_DEBUG
                target.flush();
            #endif
        }
    }
    
    
    std::string logger::make_time_string(const char* fmt) {
        auto time = std::time(nullptr);
        auto local_time = *std::localtime(&time);
        
        std::stringstream stream;
        stream << std::put_time(&local_time, fmt);
        return stream.str();
    }
    
    
    namespace loggers {
        logger& ve_logger(void) noexcept {
            static auto log_file   = io::mkdir_ofstream(io::paths::PATH_LOGS / "voxelengine.log");
            static auto log_stream = make_tee_stream(log_file, std::cout);
            
            static logger log = logger("VoxelEngine", logger::level::DEBUG, log_stream);
            return log;
        }
    }
}