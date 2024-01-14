#include <VoxelEngine/engine.hpp>

#include <vector>
#include <string>


/**
 * VELauncher entry point. Invokes the engine.
 * @param argc Number of command line arguments.
 * @param argv Command line arguments as an array of C-strings.
 * @return
 *  Returns 0 if engine::stop is called.
 *  If engine::exit is called, the program exits through std::exit, and this function does not return.
 */
int main(int argc, char** argv) {
    std::vector<std::string> args { argv, argv + argc };
    ve::get_service<ve::engine>().start(std::move(args));
}