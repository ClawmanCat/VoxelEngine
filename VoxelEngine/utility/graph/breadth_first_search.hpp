#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>
#include <VoxelEngine/utility/graph/graph.hpp>
#include <VoxelEngine/utility/graph/graph_utils.hpp>

#include <deque>


namespace ve::graph {
    /**
     * CRTP base class for visitors that can be used with @ref breadth_first_search.
     * Visitor methods may either return void, or return a visitor_state. If visitor_state::STOP is returned, the search is stopped early.
     * It is not required to override any given visitor method. You need only implement those methods that you actually need.
     */
    template <typename Derived, typename Graph> struct bfs_visitor {
        using self_type    = bfs_visitor<Derived, Graph>;
        using visitor_base = self_type;
        using graph        = Graph;
        using vertex       = vertex_of<Graph>;
        using edge         = edge_of<Graph>;


        /** Invoked when BFS visits a vertex for the first time. */
        visitor_state discover_vertex     (vertex v, graph& g) { VE_IMPL_GRAPH_CRTP_VISITOR_METHOD(self_type, Derived, discover_vertex,      (v, g), visitor_state::CONTINUE); }
        /** Invoked when BFS visits an edge that leads to an undiscovered vertex. */
        visitor_state edge_to_new_vertex  (edge   e, graph& g) { VE_IMPL_GRAPH_CRTP_VISITOR_METHOD(self_type, Derived, edge_to_new_vertex,   (e, g), visitor_state::CONTINUE); }
        /** Invoked when BFS visits an edge that leads to an already discovered vertex. */
        visitor_state edge_to_known_vertex(edge   e, graph& g) { VE_IMPL_GRAPH_CRTP_VISITOR_METHOD(self_type, Derived, edge_to_known_vertex, (e, g), visitor_state::CONTINUE); }
        /** Invoked when there are no more vertices to visit. */
        visitor_state end_reached         (graph& g)           { VE_IMPL_GRAPH_CRTP_VISITOR_METHOD(self_type, Derived, end_reached,          (g),    visitor_state::CONTINUE); }
    };

    /** Concept for classes that derive from @ref bfs_visitor. */
    template <typename T, typename G> concept bfs_visitor_for = std::is_base_of_v<bfs_visitor<T, G>, T>;


    /**
     * Performs a breadth-first-search (BFS) of the given graph, starting at the given root vertex.
     * @param graph The graph the root vertex is a part of.
     * @param root The vertex to start searching at.
     * @param visitor A @ref bfs_visitor visitor to visit the traversed graph vertices with.
     * @return visitor_state::CONTINUE if the search was completed, or visitor_state::STOP if the search was returned early
     *  by the visitor returning visitor_state::STOP when one of its callback methods was invoked.
     */
    template <graph_type G, bfs_visitor_for<G> V> inline visitor_state breadth_first_search(G& graph, vertex_of<G> root, V& visitor) {
        using vertex = vertex_of<G>;

        std::deque<vertex> queue { root };
        hash_set<vertex> seen { root };

        // Cast to base, so we can use the return type wrapper.
        typename V::visitor_base& v = visitor;

        while (!queue.empty()) {
            vertex next = take_front(queue);
            if (auto state = v.discover_vertex(next, graph); state == visitor_state::STOP) return state;

            for (auto edge : graph.get_edges(next)) {
                if (seen.contains(edge.second)) {
                    if (auto state = v.edge_to_known_vertex(edge, graph); state == visitor_state::STOP) return state;
                } else {
                    if (auto state = v.edge_to_new_vertex(edge, graph); state == visitor_state::STOP) return state;

                    seen.insert(edge.second);
                    queue.push_back(edge.second);
                }
            }
        }

        return v.end_reached(graph);
    }
}