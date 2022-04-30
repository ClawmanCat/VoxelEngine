Additional settings for Conan can be placed in this folder. 
Such settings may be provided in the form of a Conan profile (`.conanprofile` file), or a list of semicolon-separated command line settings to be passed to Conan when it is invoked (`.conansettings` file).  
Settings files should be named as `<OS>_<CONFIGURATION>.<FILETYPE>` (all lowercase), e.g. `windows_debug.conanprofile` to set the Conan profile in debug mode on Windows.  
Provided profiles currently change the following settings:

On Windows:
- Force usage of MSVC to compile dependencies, since some libraries don't like Clang.