# Getting Started (Game Development)
To create a new game, create a new CMake target using the engine's `CREATE_TARGET` function:

```cmake
INCLUDE(create_target)


CREATE_TARGET(
    MyGame
    # Version
    0 0 1
    # Shared Library (Can also be EXECUTABLE)
    SHARED 
    # Mark the target as a VoxelEngine component (i.e. not a plugin) 
    COMPONENT
    # Dependencies
    PUBLIC VoxelEngine
    PUBLIC MyAdditionalDependency
)
```

The engine requires that the game implement several callback methods, which will be invoked during the various phases of the engine's lifetime:

```c++
namespace ve::game_callbacks {
    void pre_init (void) { /* Do something here... */ }
    void post_init(void) { /* Do something here... */ }
    void pre_loop (void) { /* Do something here... */ }
    void post_loop(void) { /* Do something here... */ }
    void pre_exit (void) { /* Do something here... */ }
    void post_exit(void) { /* Do something here... */ }
}
```

The example target `VEDemoGame` can be used as a starting point as well, and already implements all of the above.  
To start your game, simply provide your own main method (Either within your game project if it is marked as `EXECUTABLE`, or within a separate launcher application) and start the `ve::engine` service:

```c++
#include <VoxelEngine/engine.hpp>

#include <vector>
#include <string>


int main(int argc, char** argv) {
    std::vector<std::string> args { argv, argv + argc };
    ve::get_service<ve::engine>().start(std::move(args));
}
```