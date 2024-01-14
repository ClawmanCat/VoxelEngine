#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/utility/graph/graph_definition.hpp>
#include <VoxelEngine/utility/graph/graph.hpp>
#include <VoxelEngine/utility/graph/graph_utils.hpp>
#include <VoxelEngine/utility/graph/find_cycles.hpp>


using namespace ve::testing;


TEST(find_cycles, non_cyclic_all_sccs) {
    auto vertices = make_graph_storage(non_cyclic_graph_edges, 10);
    auto graph    = make_graph(vertices);
    auto cycles   = ve::graph::find_strongly_connected_components(graph);

    ASSERT_EQ(cycles.size(), vertices.size());
    ASSERT_TRUE(ranges::all_of(cycles, [] (const auto& c) { return c.size() == 1; }));

    std::vector<std::size_t> vertex_ids = cycles | ve::views::join | ve::views::indirect | ve::views::transform(&vertex::id) | ranges::to<std::vector>;
    ranges::sort(vertex_ids);

    ASSERT_RANGE_EQ(vertex_ids, ve::views::iota(0, 10));
}


TEST(find_cycles, non_cyclic_non_insolated_sccs) {
    auto vertices = make_graph_storage(non_cyclic_graph_edges, 10);
    auto graph    = make_graph(vertices);
    auto cycles   = ve::graph::find_cycles(graph);

    ASSERT_TRUE(cycles.empty());
}


TEST(find_cycles, cyclic_all_sccs) {
    auto vertices = make_graph_storage(cyclic_graph_edges, 10);
    auto graph    = make_graph(vertices);
    auto cycles   = ve::graph::find_strongly_connected_components(graph);
    ranges::sort(cycles, ranges::greater{}, &std::vector<const vertex*>::size);

    auto cycle = cycles.front() | ve::views::indirect | ve::views::transform(&vertex::id) | ranges::to<std::vector>;
    ranges::sort(cycle);

    ASSERT_RANGE_EQ(cycle, std::array { 0, 3, 6 });
    ASSERT_TRUE(ranges::all_of(cycles | ve::views::drop(1), [] (const auto& c) { return c.size() == 1; }));
}


TEST(find_cycles, cyclic_non_insolated_sccs) {
    auto vertices = make_graph_storage(cyclic_graph_edges, 10);
    auto graph    = make_graph(vertices);
    auto cycles   = ve::graph::find_cycles(graph);

    ASSERT_EQ(cycles.size(), 1);

    std::vector<std::size_t> vertex_ids = cycles.front() | ve::views::indirect | ve::views::transform(&vertex::id) | ranges::to<std::vector>;
    ranges::sort(vertex_ids);

    auto cycle = cycles.front() | ve::views::indirect | ve::views::transform(&vertex::id) | ranges::to<std::vector>;
    ranges::sort(cycle);

    ASSERT_RANGE_EQ(cycle, std::array { 0, 3, 6 });
}