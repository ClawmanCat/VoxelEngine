# VoxelEngine
A game engine for voxel games, written in C++.


### Setup
On Windows, MSVC is required to compile the dependencies, since many libraries incorrectly assume 
either `Windows = MSVC` or `Clang = Linux / OSX`. This is handled automatically by the Conan profile, 
but you must have MSVC installed.  
For the engine itself, any compiler (With C++20 support) should work in theory, but only Clang has been tested currently,
and other compilers may lack the required C++20 features.  
To install the dependencies, you will need Conan (`pip install conan`). 
It is recommended to use Ninja as the generator, as other generators have not been tested.  
The Vulkan SDK is required to build the engine, even if you're compiling with `VE_GRAPHICS_API=opengl`.

To build the project (with Ninja):
```
mkdir out
cd out
cmake -DCMAKE_BUILD_TYPE=[DEBUG|RELEASE] -G Ninja -DCMAKE_C_COMPILER=[clang++|clang-cl] -DCMAKE_CXX_COMPILER=[clang++|clang-cl] ../
cmake --build ./[debug|release] --target all
```
(or use an IDE like a normal person.)


### Getting Started (Engine Development)
The file `ARCHITECTURE.md` contains an overview of how the game engine works. It is a good starting place for those wanting
to modify the engine code.


### Getting Started (Game Development)
The target `VEDemoGame` offers an example of how to use the engine to create a simple game. 
It might also be worth reading `ARCHITECTURE.md` to get an overview of what features the engine offers.


### Getting Started (Plugin Development)
The target `VEDemoPlugin` offers an example of how to use the engine to create a simple plugin for `VEDemoGame`.
It might also be worth reading `ARCHITECTURE.md` to get an overview of what features the engine offers.