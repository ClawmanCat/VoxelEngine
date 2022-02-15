#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/stack_polymorph.hpp>


namespace ve {
    // Component that invokes the given function when the entity is destroyed.
    template <typename Fn> class raii_component {
    public:
        using non_synchronizable_tag = void;


        raii_component(Fn fn) : storage(std::move(fn)) {}

        raii_component(const raii_component&) = delete;
        raii_component& operator=(const raii_component&) = delete;


        raii_component(raii_component&& other) noexcept : storage(std::move(other.storage)) {}

        raii_component& operator=(raii_component&& other) noexcept {
            storage = std::move(other.storage);
            return *this;
        }


        ~raii_component(void) {
            if (auto* fn = storage.get(); fn) std::invoke(*fn);
        }
    private:
        // The most common use case for this class is with lambdas, but due to some ambiguity in the standard,
        // most compilers currently seem to consider capturing lambdas non move-assignable.
        // Move assignability is however required for all components used with entt.
        // Using stack_polymorph here allows us to move the raii_component without resorting to heap storage.
        stack_polymorph<Fn, sizeof(Fn)> storage;
    };
}