// Prevents engine files from not having the correct resolve contexts in CLion when they are not yet included in any TU.
#ifdef VE_IDE_PASS
    #include <VoxelEngine/utility/debug/ide/clion_resolve_context_fix.hpp>
#endif