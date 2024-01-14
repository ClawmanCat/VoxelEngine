#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/graph/graph.hpp>

#include <range/v3/view.hpp>


namespace ve::graph {
    /**
     * Returns the roots of the given directed graph. That is, all vertices of the graph which have no edges going into them.
     * @param graph The graph to find the roots of.
     * @return The roots of the given graph as a vector of vertices.
     */
    template <graph_type G> inline std::vector<vertex_of<G>> find_roots(G& graph) {
        using vertex = vertex_of<G>;
        using edge   = edge_of<G>;


        hash_map<vertex, std::size_t> in_edge_count;

        for (vertex v : graph.get_vertices()) {
            for (auto [from, to] : graph.get_edges(v)) in_edge_count[to]++;
        }

        return in_edge_count
            | views::filter([] (const auto& pair) { return pair.second == 0; })
            | ranges::to<std::vector>;
    }
}