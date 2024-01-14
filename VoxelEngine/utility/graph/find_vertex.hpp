#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/graph/breadth_first_search.hpp>
#include <VoxelEngine/utility/graph/depth_first_search.hpp>


namespace ve::graph {
    namespace detail {
        /** Generic visitor to find a vertex using a predicate. Visitor base class is provided as a template parameter. */
        template <typename Pred, template <typename...> typename Base, typename... Args> struct predicate_visitor
            : Base<predicate_visitor<Pred, Base, Args...>, Args...>
        {
            using base   = Base<predicate_visitor<Pred, Base, Args...>, Args...>;
            using graph  = typename base::graph;
            using vertex = typename base::vertex;
            using edge   = typename base::edge;


            Pred* predicate;
            vertex found_vertex;


            visitor_state discover_vertex(vertex v, graph& g) {
                if (std::invoke((*predicate)(v, g))) {
                    found_vertex = v;
                    return visitor_state::STOP;
                }

                return visitor_state::CONTINUE;
            }
        };
    }


    /**
     * Returns the first (as traversed using breadth first search) vertex for which pred(vertex, graph) is true, or nullptr otherwise.
     * @param g A graph to search
     * @param root The root vertex to search from
     * @param pred A predicate invocable as pred(vertex, graph)
     * @return Either the first vertex for which pred returned true, or nullptr if no such vertex existed.
     */
    template <typename Pred, graph_type Graph> inline vertex_of<Graph> find_vertex_bfs(Graph& g, vertex_of<Graph> root, Pred pred) {
        auto visitor = detail::predicate_visitor<Pred, bfs_visitor, Graph> {
            .predicate    = std::addressof(pred),
            .found_vertex = nullptr
        };


        breadth_first_search(g, root, visitor);
        return visitor.found_vertex;
    }


    /**
     * Returns the first (as traversed using depth first search) vertex for which pred(vertex, graph) is true, or nullptr otherwise.
     * @param g A graph to search
     * @param root The root vertex to search from
     * @param pred A predicate invocable as pred(vertex, graph)
     * @return Either the first vertex for which pred returned true, or nullptr if no such vertex existed.
     */
    template <typename Pred, graph_type Graph> inline vertex_of<Graph> find_vertex_dfs(Graph& g, vertex_of<Graph> root, Pred pred) {
        auto visitor = detail::predicate_visitor<Pred, dfs_visitor, Graph> {
            .predicate    = std::addressof(pred),
            .found_vertex = nullptr
        };


        depth_first_search(g, root, visitor);
        return visitor.found_vertex;
    }
}