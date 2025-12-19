#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <exception>
#include "../coordinate2.h"
#include "../simplegraph.h"

using namespace testing;
using std::string;
using std::vector;
using std::map;
using std::pair;

class SimpleGraphTestFixture : public Test
{
protected:
    SimpleGraph graph;
    // Default constructor
};

TEST_F(SimpleGraphTestFixture, SimpleGraphAddVertexWorks)
{
    int id = graph.add_vertex(Coordinate2(0,0));
    EXPECT_EQ(0,id);

    id = graph.add_vertex(Coordinate2(0,0));
    EXPECT_EQ(0,id);  // Doesn't add anything

    id = graph.add_vertex(Coordinate2(0,5));
    EXPECT_EQ(1,id);

    vector<int> adj_vtxs = graph.get_reachable_vertices(0);
    EXPECT_THAT(adj_vtxs,SizeIs(0));
}

TEST_F(SimpleGraphTestFixture, SimpleGraphConnectTwoVerticesWorks)
{
    int id1 = graph.add_vertex(Coordinate2(0,0));
    int id2 = graph.add_vertex(Coordinate2(0,5));
    EXPECT_FALSE(graph.are_vertices_connected(id1,id2));
    EXPECT_THROW(graph.connect_vertices(id1,99),std::invalid_argument);
    EXPECT_THROW(graph.connect_vertices(99,id1),std::invalid_argument);
    graph.connect_vertices(id1,id2);
    EXPECT_TRUE(graph.are_vertices_connected(id1,id2));
    EXPECT_TRUE(graph.are_vertices_connected(id2,id1));
    graph.connect_vertices(id2,id1);
    EXPECT_TRUE(graph.are_vertices_connected(id1,id2));
    EXPECT_TRUE(graph.are_vertices_connected(id2,id1));
}


