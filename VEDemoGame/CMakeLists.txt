INCLUDE(create_target)
INCLUDE(create_include_header)


CREATE_TARGET(
    VEDemoGame
    0 0 1
    SHARED COMPONENT
    # Dependencies
    PUBLIC VoxelEngine
)


# Generate single-include headers.
CREATE_INCLUDE_HEADER(VEDemoGame core core.hpp)