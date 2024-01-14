#include <VoxelEngine/utility/services/logger.hpp>
#include <VoxelEngine/utility/io/paths.hpp>

#include <boost/preprocessor.hpp>

#include <format>
#include <iostream>
#include <ctime>
#include <iomanip>


namespace ve {
    void logger::message_impl(std::string_view message, level message_level, bool info) {
        if (message_level < logged_level) return;

        atomically([&] (logger& self) {
            auto time      = std::time(nullptr);
            auto localtime = std::localtime(&time);

            self << std::put_time(localtime, "%a, %F %T") << " [" << name << " | " << magic_enum::enum_name(message_level) << "] " << message << "\n";
        });
    }


    engine_logger::engine_logger(void) :
        logger(
            "VoxelEngine",
            VE_DEBUG_ONLY(logger::DEBUG) VE_RELEASE_ONLY(logger::INFO),
            std::cout,
            filestream
        ),
        filestream(io::paths::PATH_LOGS / "voxelengine.log")
    {}


    gfxapi_logger::gfxapi_logger(void) :
        logger(
            BOOST_PP_STRINGIZE(VE_GRAPHICS_API),
            VE_DEBUG_ONLY(logger::DEBUG) VE_RELEASE_ONLY(logger::INFO),
            std::cout,
            filestream
        ),
        filestream(io::paths::PATH_LOGS / std::format("{}.log", BOOST_PP_STRINGIZE(VE_GRAPHICS_API)))
    {}
}