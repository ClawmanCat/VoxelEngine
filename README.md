# ![Logo](./out_dirs/assets/meta/logo_small.png) VoxelEngine
A game engine for procedurally generated voxel games, written in C++.

![License](https://img.shields.io/badge/License-CC--NC--SA%204.0-brightgreen) &nbsp; &nbsp;
![Release Status](https://img.shields.io/badge/Status-PreAlpha-brightgreen) &nbsp; &nbsp;
![Version](https://img.shields.io/github/v/release/ClawmanCat/VoxelEngine?color=brightgreen&include_prereleases&label=Version)


### Getting Started
To build the engine Clang 14 or later is required (Other compilers either currently lack required C++20 functionality, or may crash while trying to parse some template logic used by the engine).  
Additionally, MSVC must be installed when building for Windows, as some dependencies expect MSVC on Windows or assume the platform is Linux if Clang is detected.  
While the engine should support other platforms than Windows, they have not yet been tested, so your results may vary.


Additionally you must have the following installed:
- [Conan](https://conan.io/) (and therefore [Python](https://www.python.org/downloads/)) to install the required dependencies (`pip install conan`).
- [CMake](https://cmake.org/download/) and some generator for CMake to build the project with (e.g. [Ninja](https://ninja-build.org/)).

To build the project (with Ninja):
```
mkdir out
cd out
cmake -DCMAKE_BUILD_TYPE=[DEBUG|RELEASE] -G Ninja -DVE_GRAPHICS_API=opengl -DCMAKE_C_COMPILER=[clang++|clang-cl] -DCMAKE_CXX_COMPILER=[clang++|clang-cl] ../
cmake --build ./[debug|release] --target all
```
(Of course you can also just use any IDE that supports CMake.)

After building the project, you can run the demonstrator game by calling VELauncher with either `--client`, `--server` or both to run either the client, the server or both at the same time. If the server is running on a different machine, you can also pass `--remote_address=[hostname]` when running the client to connect to it.

### Documentation
Documentation about how to use the engine is available at the [wiki](https://github.com/ClawmanCat/VoxelEngine/wiki). 
For those wishing to modify the engine code itself, the file `ARCHITECTURE.md` may also be of interest.  
Furthermore, this repository contains a basic example of how to set up a basic game (`./VEDemoGame`) and plugin for said game (`./VEDemoPlugin`).

### Features & Roadmap
The engine currently offers the following functionality:
- An ECS based on a [modified version](https://github.com/ClawmanCat/entt) of [Skypjack's ENTT](https://github.com/skypjack/entt).
- A highly-flexible (OpenGL) renderer capable of doing deferred PBR.
- Multiplayer support.
- Basic support for voxel spaces (procedural generation, meshing, voxel type management, etc.).
- An event system.
- An input and key-binding management system.
- A basic plugin loader.

The following features are planned (as of March 2022) but not yet implemented:
- A GUI system.
- Sounds.
- Physics.
- Extended support for voxel spaces (LODs, region system, tile entities, etc.).
- Cascaded Shadow Mapping.
- Instanced Rendering.
- Tiled Deferred Shading.
- Model Loading in various formats.
- Support for Vulkan.
- Support for Compute Shaders.
- Automatic lifetime management of plugin resources.