#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/graph/graph.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>

#include <range/v3/view.hpp>

#include <vector>


namespace ve::graph {
    template <typename Vertex> using vertex_cycle = std::vector<Vertex>;
    template <typename Vertex> using cycle_list   = std::vector<vertex_cycle<Vertex>>;


    /**
     * Finds all strongly connected components (cycles) in the given graph using Tarjan's algorithm
     * (Tarjan, R. (1972). Depth-First Search and Linear Graph Algorithms. SIAM Journal on Computing, 1(2), 146–160. doi:10.1137/0201010).
     *
     * @tparam KeepIsolated If true, all strongly connected components consisting of a single vertex are kept, otherwise they are removed.
     * @param graph A graph adapter.
     * @return A vector of vectors of vertices, where each vector forms a cycle of strongly connected components.
     */
    template <graph_type G, bool KeepIsolated = true> inline cycle_list<vertex_of<G>> find_strongly_connected_components(G& graph) {
        constexpr std::size_t no_value = max_value<std::size_t>;

        using vertex = vertex_of<G>;
        using edge   = edge_of<G>;


        struct vertex_data {
            std::size_t index    = no_value;
            std::size_t low_link = no_value;
            bool stacked         = false;
        };

        cycle_list<vertex> cycles;
        stable_hash_map<vertex, vertex_data> data;
        std::vector<vertex> stack;
        std::size_t index = 0;


        auto strongly_connected = [&] (const auto& strongly_connected, vertex v) -> void {
            auto& v_data    = data[v];
            v_data.index    = index;
            v_data.low_link = index;
            v_data.stacked  = true;

            stack.push_back(v);
            ++index;


            for (vertex w : graph.get_edges(v) | views::values) {
                auto& w_data = data[w];

                if (w_data.index == no_value) {
                    strongly_connected(strongly_connected, w);
                    v_data.low_link = std::min(v_data.low_link, w_data.low_link);
                }

                else if (w_data.stacked) {
                    v_data.low_link = std::min(v_data.low_link, w_data.index);
                }
            }


            if (v_data.low_link == v_data.index) {
                cycles.emplace_back();
                vertex w;

                do {
                    w = take_back(stack);
                    data[w].stacked = false;

                    cycles.back().push_back(w);
                } while (v != w);

                if constexpr (!KeepIsolated) {
                    if (cycles.back().size() == 1) cycles.pop_back();
                }
            }
        };


        for (vertex v : graph.get_vertices()) {
            if (data[v].index == no_value) strongly_connected(strongly_connected, v);
        }

        return cycles;
    }


    /**
     * Finds all strongly connected components of the graph with a path length more than one using Tarjan's algorithm (See @ref find_strongly_connected_components).
     * I.e. finds all cycles in a directed graph.
     *
     * @param graph A directed graph.
     * @return A vector of vectors of vertices, where every vector is a vertex cycle in the graph.
     */
    template <graph_type G> inline cycle_list<vertex_of<G>> find_cycles(G& graph) {
        return find_strongly_connected_components<G, false>(graph);
    }
}