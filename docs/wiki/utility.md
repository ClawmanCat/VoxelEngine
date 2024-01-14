### Graph Functions
While the engine has a dependency on Boost, and the `boost::graph` methods are therefor available,
these can be cumbersome to use, especially with existing datastructures.  
For this reason, the engine provides its own set of graph-traversal methods, along with a graph adaptor class
which allows any existing data structure to be easily used with these methods.

A graph adapter can be constructed by providing two methods:
- `get_vertices()` should return a view of pointers to all vertices in the graph.
- `get_edges(v)` should return a view of pairs of vertex pointers [from, to] for a pointer to a given vertex v.

```c++
// Existing data structure:
struct my_vertex {
    std::vector<my_vertex*> dependencies;
    my_data vertex_data;
};

hash_map<SomeKey, my_vertex> my_vertices { ... };

// Construct a graph by providing vertex and edge views:
auto graph = ve::graph::make_graph_adapter(
    [&] { return my_vertices | ve::views::values | ve::views::addressof; },
    [] (my_vertex* v) { return v->dependencies | ve::graph::connected_vertices_to_edges(v); }
);

// Graph adapter can now be used with existing graph algorithms:
auto cycles = ve::graph::find_cycles(graph);
VE_ASSERT(cycles.empty(), "There are circular dependencies!");
```

For graph-traversal functions, the engine makes use of visitors, similar to `boost::graph`:

```c++
GraphType graph  = ...;
auto root_vertex = ...;


struct visitor : ve::graph::dfs_visitor<visitor, GraphType> {
    using graph  = GraphType;
    using vertex = ve::graph::vertex_of<graph>;
    using edge   = ve::graph::edge_of<graph>;
    
    
    vertex the_vertex_we_are_looking_for = ...;
    
    
    // Visitor may return a visitor state (CONTINUE or STOP) or void.
    // If a visitor returns STOP, the graph traversing method will return early. 
    // Returning void is equivalent to returning CONTINUE.
    // It is not required to override all visitor methods, you can provide only the ones you need.
    ve::graph::visitor_state discover_vertex(vertex v, graph& g) { 
        if (v == the_vertex_we_are_looking_for) {
            return ve::graph::visitor_state::STOP;
        }
        
        return ve::graph::visitor_state::CONTINUE;
    }
} v;


auto result = ve::graph::depth_first_search(graph, &root_vertex, v);
VE_ASSERT(result == ve::graph::visitor_state::STOP, "Vertex was not present!");
```

Currently, the engine provides the following graph utilities (in the namespace `ve::graph`):
- `graph_adapter`, `make_graph_adapter` to construct a graph adaptor from vertex-view and edge-view generator functions.
- `connected_vertices_to_edges`: a view adapter to convert a pointers-to-vertices to edges.
- `breadth_first_search` to perform [breadth first search](https://en.wikipedia.org/wiki/Breadth-first_search) using a visitor.
- `depth_first_search` to perform [depth first search](https://en.wikipedia.org/wiki/Depth-first_search) using a visitor.
- `find_strongly_connected_components` to find all strongly connected components using [Tarjan's algorithm](https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm).
- `find_cycles` to find all cycles in a graph (i.e. all strongly connected components with a path length greater than one).
- `find_vertex_bfs`, `find_vertex_dfs` to find the first vertex that matches some predicate using BFS or DFS respectively.