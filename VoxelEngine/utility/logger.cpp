#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/utility/string.hpp>

#include <magic_enum.hpp>

#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <syncstream>
#include <fstream>


namespace ve {
    void logger::message(std::string_view msg, level msg_level, bool show_info) {
        if (msg_level < logger_level) return;


        std::stringstream out_message;
    
        if (show_info) {
            out_message << logger::make_time_string() << " ";
            out_message << '[' << name << ", " << magic_enum::enum_name(msg_level) << "] ";
        }
    
        out_message << msg << '\n';
    
        {
            std::lock_guard lock { targets_mtx };
            
            for (auto& target_ref : targets) {
                auto& target = target_ref.get();
                std::osyncstream { target } << out_message.str();
    
                #ifdef VE_DEBUG
                    target.flush();
                #endif
            }
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
        constexpr inline std::string_view gfxapi_name = BOOST_PP_STRINGIZE(VE_GRAPHICS_API);


        // By constructing the loggers like this, initialization order issues are avoided.
        logger& get_ve_logger(void) {
            static auto log_filestream = std::ofstream(io::paths::PATH_LOGS / "voxelengine.log");
            
            static auto logger_object = logger {
                "VoxelEngine"s,
                logger::level::DEBUG,
                std::cout,
                log_filestream
            };
            
            return logger_object;
        }


        logger& get_gfxapi_logger(void) {
            static auto log_filestream = std::ofstream(io::paths::PATH_LOGS / cat(gfxapi_name, ".log"));

            static auto logger_object = logger {
                to_sentence_case(gfxapi_name),
                logger::level::INFO,
                std::cout,
                log_filestream
            };

            return logger_object;
        }
    }
}