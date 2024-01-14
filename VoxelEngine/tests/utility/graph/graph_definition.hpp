#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/graph/graph.hpp>
#include <VoxelEngine/utility/graph/graph_utils.hpp>


namespace ve::testing {
    struct vertex {
        std::size_t id = 0;
        std::vector<vertex*> edges {};
    };


    // ┌ -> 1 -> 4
    // 0 -> 2 -> 5
    // └ -> 3 -> 6
    // 7 -> 8 -> 9
    const std::vector<std::pair<std::size_t, std::size_t>> non_cyclic_graph_edges {
        { 0, 1 }, { 0, 2 }, { 0, 3 },
        { 1, 4 },
        { 2, 5 },
        { 3, 6 },
        { 7, 8 }, { 8, 9 }
    };


    //      ┌ -> 1 -> 4
    // ┌ -> 0 -> 2 -> 5
    // |    └ -> 3 -> 6
    // └--------------┘
    // 7 -> 8 -> 9
    const std::vector<std::pair<std::size_t, std::size_t>> cyclic_graph_edges {
        { 0, 1 }, { 0, 2 }, { 0, 3 },
        { 1, 4 },
        { 2, 5 },
        { 3, 6 }, { 6, 0 },
        { 7, 8 }, { 8, 9 }
    };


    auto make_graph_storage(const auto& edges, std::size_t num_vertices) {
        hash_map<std::size_t, vertex> vertices;

        for (std::size_t i = 0; i < num_vertices; ++i) vertices.emplace(i, vertex { .id = i });
        for (const auto& [from, to] : edges) vertices.at(from).edges.push_back(&vertices.at(to));

        return vertices;
    }


    auto make_graph(const auto& vertices) {
        auto get_vertices = [&] { return vertices | views::values | views::addressof; };
        auto get_edges    = [] (const auto* v) { return v->edges | graph::connected_vertices_to_edges(v); };

        return graph::make_graph_adapter<const vertex>(get_vertices, get_edges);
    }
}