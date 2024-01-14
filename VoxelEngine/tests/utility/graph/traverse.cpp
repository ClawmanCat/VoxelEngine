#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/utility/graph/graph_definition.hpp>
#include <VoxelEngine/utility/graph/graph.hpp>
#include <VoxelEngine/utility/graph/graph_utils.hpp>
#include <VoxelEngine/utility/graph/depth_first_search.hpp>
#include <VoxelEngine/utility/graph/breadth_first_search.hpp>


using namespace ve::testing;


template <template <typename...> typename Base, typename G>
auto make_visitor(G& graph, std::vector<std::size_t> ordering, std::vector<std::size_t> disconnected) {
    struct visitor : Base<visitor, G> {
        using base   = Base<visitor, G>;
        using vertex = typename base::vertex;
        using edge   = typename base::edge;
        using graph  = typename base::graph;

        ve::hash_set<vertex> seen;
        std::vector<std::size_t> ordering;
        std::vector<std::size_t> disconnected;


        void discover_vertex(vertex v, graph& g) {
            EXPECT_FALSE(seen.contains(v));
            seen.insert(v);

            if (!ordering.empty() && ordering.back() != v->id) {
                EXPECT_FALSE(ranges::contains(ordering, v->id)) << "Found vertex " << v->id << " before " << ordering.back();
            }

            if (!ordering.empty() && ordering.back() == v->id) {
                ordering.pop_back();
            }

            EXPECT_FALSE(ranges::contains(disconnected, v->id));
        }


        void edge_to_new_vertex(edge e, graph& g) {
            EXPECT_TRUE(seen.contains(e.first));
            EXPECT_FALSE(seen.contains(e.second));
        }


        void edge_to_known_vertex(edge e, graph& g) {
            EXPECT_TRUE(seen.contains(e.first));
            EXPECT_TRUE(seen.contains(e.second));
        }
    };


    return visitor { .ordering = std::move(ordering), .disconnected = std::move(disconnected) };
}


TEST(traverse_graph, bfs_non_cyclic) {
    auto vertices = make_graph_storage(non_cyclic_graph_edges, 10);
    auto graph    = make_graph(vertices);
    auto visitor  = make_visitor<ve::graph::bfs_visitor>(graph, { 6, 1, 0 }, { 7, 8, 9 });

    ve::graph::breadth_first_search(graph, &vertices.at(0), visitor);
    ASSERT_EQ(visitor.seen.size(), 7);
}


TEST(traverse_graph, bfs_cyclic) {
    auto vertices = make_graph_storage(cyclic_graph_edges, 10);
    auto graph    = make_graph(vertices);
    auto visitor  = make_visitor<ve::graph::bfs_visitor>(graph, { 6, 1, 0 }, { 7, 8, 9 });

    ve::graph::breadth_first_search(graph, &vertices.at(0), visitor);
    ASSERT_EQ(visitor.seen.size(), 7);
}


TEST(traverse_graph, dfs_non_cyclic) {
    auto vertices = make_graph_storage(non_cyclic_graph_edges, 10);
    auto graph    = make_graph(vertices);
    auto visitor  = make_visitor<ve::graph::dfs_visitor>(graph, { 4, 1, 0 }, { 7, 8, 9 });

    ve::graph::depth_first_search(graph, &vertices.at(0), visitor);
    ASSERT_EQ(visitor.seen.size(), 7);
}


TEST(traverse_graph, dfs_cyclic) {
    auto vertices = make_graph_storage(cyclic_graph_edges, 10);
    auto graph    = make_graph(vertices);
    auto visitor  = make_visitor<ve::graph::dfs_visitor>(graph, { 4, 1, 0 }, { 7, 8, 9 });

    ve::graph::depth_first_search(graph, &vertices.at(0), visitor);
    ASSERT_EQ(visitor.seen.size(), 7);
}