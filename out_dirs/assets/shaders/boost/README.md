Source files from Boost.Preprocessor (retrieved from the Conan package)
will be copied here when the engine is built.  
This allows shaders to use Boost.Preprocessor directly.  
Note that while this folder contains C++ headers (.hpp files), these files only contain preprocessor directives, and can thus be safely used as if they were .glsl shaders.

Boost.Preprocessor is licensed under the Boost Software License (https://www.boost.org/users/license.html).  
A copy of the license can be found in LICENSE_BOOST.md.