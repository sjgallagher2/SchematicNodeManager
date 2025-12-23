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

class SimpleGraphTestFixtureWithVertices : public Test
{
protected:
    SimpleGraph graph;
    SimpleGraphTestFixtureWithVertices() {
        id1 = graph.add_vertex({0,0});
        id2 = graph.add_vertex({0,5});
        id3 = graph.add_vertex({5,5});
        id4 = graph.add_vertex({7,5});
        id5 = graph.add_vertex({7,9});
        id6 = graph.add_vertex({2,1});
        id7 = graph.add_vertex({1,1});

        graph.connect_vertices(id1,id3);
        graph.connect_vertices(id2,id3);
        graph.connect_vertices(id3,id4);
        graph.connect_vertices(id4,id5);
        graph.connect_vertices(id4,id6);
        graph.connect_vertices(id4,id7);
        graph.connect_vertices(id6,id7);
    }
    int id1,id2,id3,id4,id5,id6,id7;
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

TEST_F(SimpleGraphTestFixture, SimpleGraphInitializesTwoVerticesCorrectly)
{
    // Can we construct vertices
    Coordinate2 p1(0,0);
    Coordinate2 p2(0,5);
    int id1 = graph.add_vertex(p1);
    int id2 = graph.add_vertex(p2);

    /* Were the vertices added properly */
    // Are these vertices connected (they should not be)
    EXPECT_FALSE(graph.are_vertices_adjacent(id1,id2));
    // Are these vertices both isolated (they should be)
    EXPECT_TRUE(graph.is_vertex_isolated(id1));
    EXPECT_TRUE(graph.is_vertex_isolated(id2));
    // Calling is_vertex_isolated on a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.is_vertex_isolated(99),std::invalid_argument);
    // Are these the only vertex id's listed (they should be)
    vector<int> vids = graph.get_vertex_ids();
    EXPECT_THAT(vids,ElementsAre(id1,id2));
    // Are the positions correct
    EXPECT_EQ(p1,graph.get_vertex_pos(id1));
    EXPECT_EQ(p2,graph.get_vertex_pos(id2));
}

TEST_F(SimpleGraphTestFixture, SimpleGraphConnectTwoVerticesWorks)
{
    Coordinate2 p1(0,0);
    Coordinate2 p2(0,5);
    int id1 = graph.add_vertex(p1);
    int id2 = graph.add_vertex(p2);

    /* Testing connections */
    // Can we connect to a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.connect_vertices(id1,99),std::invalid_argument);
    EXPECT_THROW(graph.connect_vertices(99,id1),std::invalid_argument);
    // Can we connect two existing vertices (they should become connected)
    graph.connect_vertices(id1,id2);
    EXPECT_TRUE(graph.are_vertices_adjacent(id1,id2));
    EXPECT_TRUE(graph.are_vertices_adjacent(id2,id1));
    // Are these vertices both isolated (they should not be anymore)
    EXPECT_FALSE(graph.is_vertex_isolated(id1));
    EXPECT_FALSE(graph.is_vertex_isolated(id2));
    // Is it safe to call connect_vertices() twice (it should be)
    graph.connect_vertices(id2,id1);
    EXPECT_TRUE(graph.are_vertices_adjacent(id1,id2));
    EXPECT_TRUE(graph.are_vertices_adjacent(id2,id1));
    // Can we call connect_vertices() with the same vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.connect_vertices(id1,id1),std::invalid_argument);
    EXPECT_THROW(graph.connect_vertices(id2,id2),std::invalid_argument);
}

TEST_F(SimpleGraphTestFixture, SimpleGraphDisconnectVerticesWorks)
{
    Coordinate2 p1(0,0);
    Coordinate2 p2(0,5);
    int id1 = graph.add_vertex(p1);
    int id2 = graph.add_vertex(p2);
    graph.connect_vertices(id1,id2);

    /* Testing disconnections */
    // Can we disconnect a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.disconnect_vertices(id1,99),std::invalid_argument);
    EXPECT_THROW(graph.disconnect_vertices(99,id1),std::invalid_argument);
    // Are these vertices both isolated (they should not be)
    EXPECT_FALSE(graph.is_vertex_isolated(id1));
    EXPECT_FALSE(graph.is_vertex_isolated(id2));
    // Can we disconnect two existing vertices (they should become isolated)
    graph.disconnect_vertices(id1,id2);
    EXPECT_FALSE(graph.are_vertices_adjacent(id1,id2));
    EXPECT_FALSE(graph.are_vertices_adjacent(id2,id1));
    EXPECT_TRUE(graph.is_vertex_isolated(id1));
    EXPECT_TRUE(graph.is_vertex_isolated(id2));
    // Is it safe to call disconnect_vertices() on disconnected vertices  (it should be)
    graph.disconnect_vertices(id2,id1);
    EXPECT_FALSE(graph.are_vertices_adjacent(id1,id2));
    EXPECT_FALSE(graph.are_vertices_adjacent(id2,id1));
    // Can we call disconnect_vertices() with the same vertex (should do nothing)
    EXPECT_NO_THROW(graph.disconnect_vertices(id1,id1));
    EXPECT_NO_THROW(graph.disconnect_vertices(id2,id2));
}

TEST_F(SimpleGraphTestFixtureWithVertices, SimpleGraphDeleteVerticesWorks)
{
    // Verify some connections
    EXPECT_FALSE(graph.is_vertex_isolated(id5));
    EXPECT_TRUE(graph.are_vertices_adjacent(id6,id7));
    EXPECT_TRUE(graph.are_vertices_adjacent(id1,id3));
    EXPECT_TRUE(graph.are_vertices_adjacent(id2,id3));

    // Delete 4, then 5 becomes isolated, 6 and 7 are connected, 1, 3, and 2 are connected
    graph.delete_vertex(id4);
    EXPECT_TRUE(graph.is_vertex_isolated(id5));
    EXPECT_TRUE(graph.are_vertices_adjacent(id6,id7));
    EXPECT_TRUE(graph.are_vertices_adjacent(id1,id3));
    EXPECT_TRUE(graph.are_vertices_adjacent(id2,id3));

    // Try to access a non-existent vertex
    EXPECT_THROW(graph.delete_vertex(id4),std::invalid_argument);
    EXPECT_THROW(graph.get_vertex_pos(id4),std::invalid_argument);
    EXPECT_THROW(graph.connect_vertices(id1,id4),std::invalid_argument);
    EXPECT_THROW(graph.disconnect_vertices(id4,id6),std::invalid_argument);
}

TEST_F(SimpleGraphTestFixtureWithVertices, SimpleGraphTraverseGraphExample)
{
    graph.traverse_graph();
}

