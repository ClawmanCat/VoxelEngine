#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>


namespace ve::testing {
    struct copyable_component {
        ve::u64 value;

        bool operator==(const copyable_component&) const = default;
    };


    struct noncopyable_component {
        ve::u64 value;

        noncopyable_component(void) = default;
        noncopyable_component(ve::u64 v) : value(v) {}

        VE_MOVE_ONLY(noncopyable_component);

        bool operator==(const noncopyable_component&) const = default;
    };


    struct empty_component {
        empty_component(void) = default;
        empty_component(ve::u64) {}

        bool operator==(const empty_component&) const = default;
    };


    struct noncopyable_empty_component {
        noncopyable_empty_component(void) = default;
        noncopyable_empty_component(ve::u64) {}

        VE_MOVE_ONLY(noncopyable_empty_component);

        bool operator==(const noncopyable_empty_component&) const = default;
    };


    struct heap_allocating_component {
        std::vector<ve::u64> v;

        heap_allocating_component(void) : v { 1, 2, 3, 4, 5, 6 } {}
        heap_allocating_component(ve::u64 val) : v { val, val, val, val, val, val } {}

        bool operator==(const heap_allocating_component&) const = default;
    };


    using component_types = ve::meta::pack<
        copyable_component,
        noncopyable_component,
        empty_component,
        noncopyable_empty_component,
        heap_allocating_component
    >;
}