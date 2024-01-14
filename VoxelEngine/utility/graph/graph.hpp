#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/container_traits.hpp>
#include <VoxelEngine/utility/meta/is_template.hpp>


namespace ve::graph {
    /** A vertex_getter is a function-object that when invoked returns a range of pointers to vertices (The vertices of the graph). */
    template <typename Getter, typename Vertex, typename R = std::invoke_result_t<Getter>>
    concept vertex_getter = std::convertible_to<meta::container_value_type<R>, Vertex*>;

    /**
     * An index_getter is a function-object that when invoked with a pointer to a vertex, returns all edges of that vertex,
     * where an edge is a pair of pointers-to-vertices, with the first element of that pair the vertex that was passed to the function-object.
     */
    template <typename Getter, typename Vertex, typename R = std::invoke_result_t<Getter, Vertex*>>
    concept edge_getter = std::convertible_to<meta::container_value_type<R>, std::pair<Vertex*, Vertex*>>;


    /**
     * A graph_adapter provides a common interface to an existing data structure that allows it to be used with the engine's graph algorithms.
     * For a given data structure, the graph_adapter requires a method that returns a range of all vertices (GetVertices) and all edges of a given vertex (GetEdges).
     * The graph adapter uses Vertex* as its vertex type and std::pair<Vertex*, Vertex*> as its edge type, so vertices and edges are cheaply copyable.
     *
     * @tparam Vertex The type of vertex used with this graph.
     * @tparam GetVertices A function-object to get the vertices of the graph, fulfilling the concept @ref vertex_getter.
     * @tparam GetEdges A function-object to get the edges of a given vertex, fulfilling the concept @ref edge_getter.
     */
    template <typename Vertex, vertex_getter<Vertex> GetVertices, edge_getter<Vertex> GetEdges> struct graph_adapter {
        using vertex_type      = Vertex*;
        using edge_type        = std::pair<Vertex*, Vertex*>;
        using vertex_getter_fn = GetVertices;
        using edge_getter_fn   = GetEdges;


        GetVertices get_vertices;
        GetEdges get_edges;
    };


    /** Utility function to construct a @ref graph_adapter, deducing its vertex type from the provided getter functions. */
    template <typename GetVertices, typename GetEdges>
    constexpr inline auto make_graph_adapter(GetVertices get_vertices, GetEdges get_edges) {
        using range_type  = std::invoke_result_t<GetVertices>;
        using vertex_type = std::remove_pointer_t<meta::container_value_type<range_type>>;

        return graph_adapter<vertex_type, GetVertices, GetEdges> {
            .get_vertices = std::move(get_vertices),
            .get_edges    = std::move(get_edges)
        };
    }


    /** Utility function to construct a @ref graph_adapter with the provided vertex type. */
    template <typename Vertex, typename GetVertices, typename GetEdges>
    constexpr inline auto make_graph_adapter(GetVertices get_vertices, GetEdges get_edges) {
        return graph_adapter<Vertex, GetVertices, GetEdges> {
            .get_vertices = std::move(get_vertices),
            .get_edges    = std::move(get_edges)
        };
    }


    /** Concept for types that can be used with engine graph algorithms. */
    template <typename T> concept graph_type = meta::is_template_v<graph_adapter, T>;

    template <graph_type G> using vertex_of = typename G::vertex_type;
    template <graph_type G> using edge_of   = typename G::edge_type;
}