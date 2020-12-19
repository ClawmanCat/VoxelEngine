# VoxelEngine
A game engine for voxel games, written in C++.


### Setup
On Windows, MSVC is required to compile the dependencies, since many libraries incorrectly assume 
either `Windows = MSVC` or `Clang = Linux`. This is handled automatically by the conan profile, 
but you must have MSVC installed.  
For the engine itself, any compiler (With C++20 support) should work in theory, but only Clang has been tested currently.