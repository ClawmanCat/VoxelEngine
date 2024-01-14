#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/graph/graph.hpp>
#include <VoxelEngine/utility/graph/find_roots.hpp>
#include <VoxelEngine/utility/graph/breadth_first_search.hpp>


namespace ve::graph {
    template <typename G> using distance_map = hash_map<vertex_of<G>, std::size_t>;


    template <graph_type G> inline distance_map<G> distance_from_root(G& graph) {
        using vertex = vertex_of<G>;


        struct visitor : bfs_visitor<visitor, G> {
            distance_map<G> result;


            visitor_state discover_vertex(vertex v, G& g) {

            }
        } v;


        for (vertex root : find_roots(graph)) breadth_first_search(graph, root, v);
    }
}