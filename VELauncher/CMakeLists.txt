include(create_target)

create_target(
    VELauncher
    EXECUTABLE
    0 0 1
    # Dependencies:
    PRIVATE VEDemoGame
)


target_compile_definitions(VELauncher PRIVATE "VE_BUILD_LAUNCHER=1")