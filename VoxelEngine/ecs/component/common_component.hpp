#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::detail {
    // ENTT does not support views without any components. Some systems may require these however.
    // To resolve this, this empty component is added to all entities, and can be used instead of an empty view.
    // Note: the view wrapper class automatically converts empty views to views of this component.
    struct common_component {
        using non_removable_tag = void;
        using non_syncable_tag  = void;
    };
}