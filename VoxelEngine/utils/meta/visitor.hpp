#pragma once

#include <VoxelEngine/core/core.hpp>

#include <utility>


namespace ve::meta {
    template <typename... Fns>
    class visitor : public Fns... {
    public:
        visitor(Fns&&... fns) : Fns(std::forward<Fns>(fns))... {}
        
        using Fns::operator()...;
    };
}