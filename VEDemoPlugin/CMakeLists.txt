include(create_target)
include(generate_include_header)


create_target(
    VEDemoPlugin
    SHARED
    0 0 1
    # Dependencies:
    PUBLIC VEDemoGame
)

# Plugins should output into the plugin directory.
set_target_properties(VEDemoPlugin PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/plugins/VEDemoPlugin")
set_target_properties(VEDemoPlugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/plugins/VEDemoPlugin")
set_target_properties(VEDemoPlugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/VEDemoPlugin")


generate_include_header(VEDemoPlugin core core.hpp)