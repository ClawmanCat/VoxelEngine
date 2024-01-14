# ![Logo](VoxelEngine/assets/logo_small.png) VoxelEngine
A game engine for procedurally generated voxel games, written in C++.

### Getting Started
To build the engine, a recent build of Clang (>=16.0.0) is required. Other compilers are planned to be supported in the future and may or may not be able to compile the engine currently.  
Additionally, MSVC (>=17.7.3) is required to build the dependencies on Windows, as some of the engine's dependencies will not build with Clang, as they assume either Windows = MSVC or Clang = Linux.

Furthermore, you must have the following installed:
- [Python](https://www.python.org/downloads/) to run Conan and some parts of the build system.
- [Conan](https://conan.io/) to install the required dependencies (`pip install conan`). Note that Conan 2 is currently not supported.
- [CMake](https://cmake.org/download/) and some generator for CMake to build the project with (e.g. [Ninja](https://ninja-build.org/)).
- On Windows, [SymbolGenerator]() will be used to manage the symbols exported within the engine/game DLL. This program will be downloaded automatically when the build system is run.

To build the project (with Ninja):
```
mkdir out
cd out
cmake -DCMAKE_BUILD_TYPE=[DEBUG|RELEASE|RELWITHDEBINFO] -G Ninja -DVE_GRAPHICS_API=opengl -DCMAKE_C_COMPILER=[clang++|clang-cl] -DCMAKE_CXX_COMPILER=[clang++|clang-cl] ../
cmake --build ./[debug|release] --target all
```

### Configuration
The engine supports several configuration parameters, either as CMake configuration options or as preprocessor directives which may be defined.

<table>
    <caption>CMake Options</caption>
    <th>Option</th>
    <th>Value Type</th>
    <th>Function</th>
    <tr>
        <td><code>VE_ENABLE_TESTING</code></td>
        <td>Boolean</td>
        <td>If true, CMake targets for unit test targets will be generated.</td>
    </tr>
    <tr>
        <td><code>VE_GRAPHICS_API</code></td>
        <td>One of: OpenGL, Vulkan</td>
        <td>Sets what graphics API will be used by the engine. Note that Vulkan support is currently not yet supported.</td>
    </tr>
    <tr>
        <td><code>VE_PROFILE_NAME</code></td>
        <td>Valid folder name</td>
        <td>Sets the name of the current profile. This is the folder build artifacts will be output to. The default is <code>{compiler name}-{build type}</code>.</td>
    </tr>
    <tr>
        <td><code>VE_COMPILER_PROFILE</code></td>
        <td>Boolean</td>
        <td>If true, a preconfigured profile from <code>./cmake/profiles</code> will be used to enable extra compiler warnings.</td>
    </tr>
    <tr>
        <td><code>VE_WARNINGS_ARE_ERRORS</code></td>
        <td>Boolean</td>
        <td>If true, all compiler warnings are treated as errors. This option requires <code>VE_COMPILER_PROFILE</code> to be true.</td>
    </tr>
</table>

<table>
    <caption>Preprocessor Options</caption>
    <th>Option</th>
    <th>Value Type</th>
    <th>Function</th>
    <tr>
        <td><code>VE_ENABLE_DEBUGGING</code></td>
        <td><code>ifdef</code></td>
        <td>If true, engine debug asserts will be generated, even in release builds.</td>
    </tr>
    <tr>
        <td><code>VE_IDE_PASS</code></td>
        <td><code>ifdef</code></td>
        <td>If true, some parts of the engine which are hard to parse for tools like IntelliSense will be disabled. Do not use this to actually build the engine.</td>
    </tr>
    <tr>
        <td><code>VE_DECOMPOSER_LIMIT</code></td>
        <td>integer</td>
        <td>Maximum number of elements that can be decomposed using the structured-binding decomposer. See <code>VoxelEngine/utility/decompose/generated_decomposer.hpp</code>.<br>Defaults to 64.</td>
    </tr>
    <tr>
        <td><code>VE_DEFAULT_ENTITY_TYPE</code></td>
        <td>typename (unsigned integral)</td>
        <td>Default entity type used within the ECS if no other type is specified.<br>Defaults to <code>ve::u64</code>.</td>
    </tr>
    <tr>
        <td><code>VE_DEFAULT_ENTITY_UBITS</code></td>
        <td>integer</td>
        <td>Default number of bits within entity IDs which are left free for user-defined purposes.<br>Defaults to 1. This bit can be used in multiplayer environments to generate client-side entity IDs.</td>
    </tr>
</table>