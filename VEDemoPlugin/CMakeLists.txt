INCLUDE(create_target)
INCLUDE(create_include_header)


CREATE_TARGET(
    VEDemoPlugin
    0 0 1
    SHARED PLUGIN
    # Dependencies
    PUBLIC VEDemoGame
)


# Generate single-include headers.
CREATE_INCLUDE_HEADER(VEDemoPlugin core core.hpp)