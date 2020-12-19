include(create_target)

create_target(
    VEDemoGame
    SHARED
    0 0 1
    # Dependencies:
    PUBLIC VoxelEngine
)

# Indicates that the game is being built, and not some dependency like a plugin.
target_compile_definitions(VEDemoGame PRIVATE VE_BUILD_GAME)