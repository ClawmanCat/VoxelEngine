include(create_target)

create_target(
    VEDemoPlugin
    SHARED
    0 0 1
    # Dependencies:
    PRIVATE VEDemoGame
)

# Plugins go in the plugin directory.
set_target_properties(
        VEDemoPlugin
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins"
)