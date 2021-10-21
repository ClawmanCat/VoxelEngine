include(create_target)
include(generate_include_header)


create_target(
    VEDemoGame
    SHARED
    0 0 1
    # Dependencies:
    PUBLIC VoxelEngine
)

# Allow automatic selection of the actor when making actor-dependent function calls.
target_compile_definitions(VEDemoGame PRIVATE VE_BUILD_GAME)