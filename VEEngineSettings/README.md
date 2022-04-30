# VoxelEngine Settings
The engine provides several settings whose value must be directly visible to all translation units during engine compilation,
like settings that control what type to use for certain things, or values that must be provided in a constexpr context.  
If the engine was a header-only library, this could simply be accomplished by defining these settings before including the engine,
but since the engine has its own translation units, this does not work.
Instead, this settings target exists, which includes the file `preinclude.hpp`, which is included in every engine translation unit.  
Since this header is included by the engine everywhere, it may not itself include any engine headers. Engine headers may however be included from `preinclude.cpp`.  
  
Note that the engine must be recompiled in order for changes to these settings to take effect.