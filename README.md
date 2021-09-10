# Cmake + Conan Project Template
A template for a C++ project using CMake and Conan.

### Usage
You should install CMake (3.19 or newer), Conan (available through pip) and have some generator (Visual Studio, Ninja, Makefile, etc.) installed.  
To set up and build the project (with Ninja):
```shell
mkdir out
cd out

cmake -DCMAKE_BUILD_TYPE=[DEBUG|RELEASE] -G Ninja -DCMAKE_C_COMPILER=[Compiler] -DCMAKE_CXX_COMPILER=[Compiler] ../
cmake --build ./[debug|release] --target all
```
(or just call CMake through your IDE like a normal person.)