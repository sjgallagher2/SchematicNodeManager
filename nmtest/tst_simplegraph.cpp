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

class SimpleVertexGraphTestFixture : public Test
{
protected:
    SimpleVertexGraph graph;
};

class SimpleGraphTestFixtureWithVertices : public Test
{
protected:
    SimpleGraph graph;
    SimpleGraphTestFixtureWithVertices() {
        id0 = graph.add_node(false);
        id1 = graph.add_node(false);
        id2 = graph.add_node(false);
        id3 = graph.add_node(false);
        id4 = graph.add_node(false);
        id5 = graph.add_node(false);
        id6 = graph.add_node(false);
        id7 = graph.add_node(false);

        graph.connect_nodes(id1,id3,false);
        graph.connect_nodes(id2,id3,false);
        graph.connect_nodes(id3,id4,false);
        graph.connect_nodes(id4,id5,false);
        graph.connect_nodes(id4,id6,false);
        graph.connect_nodes(id4,id7,false);
        graph.connect_nodes(id6,id7,true);  // with traversal
    }
    int id0,id1,id2,id3,id4,id5,id6,id7;
};


class SimpleVertexGraphTestFixtureWithVertices : public Test
{
protected:
    SimpleVertexGraph graph;
    SimpleVertexGraphTestFixtureWithVertices() {
        id0 = graph.add_vertex({-1,0},false);
        id1 = graph.add_vertex({0,0},false);
        id2 = graph.add_vertex({1,0},false);
        id3 = graph.add_vertex({2,0},false);
        id4 = graph.add_vertex({3,0},false);
        id5 = graph.add_vertex({4,0},false);
        id6 = graph.add_vertex({5,0},false);
        id7 = graph.add_vertex({6,0},false);

        graph.connect_nodes(id1,id3,false);
        graph.connect_nodes(id2,id3,false);
        graph.connect_nodes(id3,id4,false);
        graph.connect_nodes(id4,id5,false);
        graph.connect_nodes(id4,id6,false);
        graph.connect_nodes(id4,id7,false);
        graph.connect_nodes(id6,id7,true);  // with traversal
    }
    int id0,id1,id2,id3,id4,id5,id6,id7;
};


// First test the supporting id pool data structure
TEST(GraphIdPoolSuite, TestGraphIdPoolFunctionality)
{
    IdPool pool;
    int id0,id1,id2;
    id0 = pool.get();
    id1 = pool.get();
    id2 = pool.get();
    pool.put_back(id1);
    id1 = pool.get();     // should get its id back
    EXPECT_EQ(1,id1);
    EXPECT_THROW(pool.put_back(-1),std::out_of_range);
    EXPECT_THROW(pool.put_back(99),std::out_of_range);
}

TEST_F(SimpleGraphTestFixture, SimpleGraphAddNodeWorks)
{
    int id = graph.add_node();
    EXPECT_EQ(0,id);
    id = graph.add_node();
    EXPECT_EQ(1,id);
    id = graph.add_node();
    EXPECT_EQ(2,id);
    vector<int> adj_vtxs = graph.get_reachable_nodes(id);
    EXPECT_THAT(adj_vtxs,ElementsAre(id));
}

TEST_F(SimpleVertexGraphTestFixture, SimpleVertexGraphAddVertexWorks)
{
    int id = graph.add_vertex(Coordinate2(0,0));
    EXPECT_EQ(0,id);
    id = graph.add_vertex(Coordinate2(0,0));
    EXPECT_EQ(0,id);  // Doesn't add anything
    id = graph.add_vertex(Coordinate2(0,5));
    EXPECT_EQ(1,id);
    vector<int> adj_vtxs = graph.get_reachable_nodes(id);
    EXPECT_THAT(adj_vtxs,ElementsAre(id));
}

TEST_F(SimpleGraphTestFixture, SimpleGraphInitializesTwoVerticesCorrectly)
{
    // Can we construct nodes
    int id1 = graph.add_node();
    int id2 = graph.add_node();

    /* Were the vertices added properly */
    // Are these vertices connected (they should not be)
    // Throw invalid_argument if either id does not exist
    EXPECT_THROW(graph.are_nodes_adjacent(99,0),std::invalid_argument);
    EXPECT_THROW(graph.are_nodes_adjacent(0,99),std::invalid_argument);
    EXPECT_FALSE(graph.are_nodes_adjacent(id1,id2));
    // Are these vertices both isolated (they should be)
    EXPECT_TRUE(graph.is_node_isolated(id1));
    EXPECT_TRUE(graph.is_node_isolated(id2));
    // Calling is_vertex_isolated on a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.is_node_isolated(99),std::invalid_argument);
    // Are these the only vertex id's listed (they should be)
    vector<int> vids = graph.get_all_node_ids();
    EXPECT_THAT(vids,ElementsAre(id1,id2));
}

TEST_F(SimpleVertexGraphTestFixture, SimpleVertexGraphInitializesTwoVerticesCorrectly)
{
    // Can we construct vertices
    Coordinate2 p1(0,0);
    Coordinate2 p2(0,5);
    int id1 = graph.add_vertex(p1);
    int id2 = graph.add_vertex(p2);

    /* Were the vertices added properly */
    // Are these vertices connected (they should not be)
    // Throw invalid_argument if either id does not exist
    EXPECT_THROW(graph.are_nodes_adjacent(99,0),std::invalid_argument);
    EXPECT_THROW(graph.are_nodes_adjacent(0,99),std::invalid_argument);
    EXPECT_FALSE(graph.are_nodes_adjacent(id1,id2));
    // Are these vertices both isolated (they should be)
    EXPECT_TRUE(graph.is_node_isolated(id1));
    EXPECT_TRUE(graph.is_node_isolated(id2));
    // Calling is_vertex_isolated on a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.is_node_isolated(99),std::invalid_argument);
    // Are these the only vertex id's listed (they should be)
    vector<int> vids = graph.get_all_node_ids();
    EXPECT_THAT(vids,ElementsAre(id1,id2));
}

TEST_F(SimpleGraphTestFixture, SimpleGraphConnectTwoVerticesWorks)
{
    int id1 = graph.add_node();
    int id2 = graph.add_node();

    /* Testing connections */
    // Can we connect to a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.connect_nodes(id1,99),std::invalid_argument);
    EXPECT_THROW(graph.connect_nodes(99,id1),std::invalid_argument);
    // Can we connect two existing vertices (they should become connected)
    graph.connect_nodes(id1,id2);
    EXPECT_TRUE(graph.are_nodes_adjacent(id1,id2));
    EXPECT_TRUE(graph.are_nodes_adjacent(id2,id1));
    // Are these vertices both isolated (they should not be anymore)
    EXPECT_FALSE(graph.is_node_isolated(id1));
    EXPECT_FALSE(graph.is_node_isolated(id2));
    // Is it safe to call connect_vertices() twice (it should be)
    graph.connect_nodes(id2,id1);
    EXPECT_TRUE(graph.are_nodes_adjacent(id1,id2));
    EXPECT_TRUE(graph.are_nodes_adjacent(id2,id1));
    // Can we call connect_vertices() with the same vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.connect_nodes(id1,id1),std::invalid_argument);
    EXPECT_THROW(graph.connect_nodes(id2,id2),std::invalid_argument);
}

TEST_F(SimpleGraphTestFixture, SimpleGraphDisconnectVerticesWorks)
{
    int id1 = graph.add_node();
    int id2 = graph.add_node();
    graph.connect_nodes(id1,id2);

    /* Testing disconnections */
    // Can we disconnect a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.disconnect_nodes(id1,99),std::invalid_argument);
    EXPECT_THROW(graph.disconnect_nodes(99,id1),std::invalid_argument);
    // Are these vertices both isolated (they should not be)
    EXPECT_FALSE(graph.is_node_isolated(id1));
    EXPECT_FALSE(graph.is_node_isolated(id2));
    // Can we disconnect two existing vertices (they should become isolated)
    graph.disconnect_nodes(id1,id2);
    EXPECT_FALSE(graph.are_nodes_adjacent(id1,id2));
    EXPECT_FALSE(graph.are_nodes_adjacent(id2,id1));
    EXPECT_TRUE(graph.is_node_isolated(id1));
    EXPECT_TRUE(graph.is_node_isolated(id2));
    // Is it safe to call disconnect_vertices() on disconnected vertices  (it should be)
    graph.disconnect_nodes(id2,id1);
    EXPECT_FALSE(graph.are_nodes_adjacent(id1,id2));
    EXPECT_FALSE(graph.are_nodes_adjacent(id2,id1));
    // Can we call disconnect_vertices() with the same vertex (should do nothing)
    EXPECT_NO_THROW(graph.disconnect_nodes(id1,id1));
    EXPECT_NO_THROW(graph.disconnect_nodes(id2,id2));
}

TEST_F(SimpleGraphTestFixtureWithVertices, SimpleGraphDeleteVerticesWorks)
{
    // Verify some connections
    EXPECT_FALSE(graph.is_node_isolated(id5));
    EXPECT_TRUE(graph.are_nodes_adjacent(id6,id7));
    EXPECT_TRUE(graph.are_nodes_adjacent(id1,id3));
    EXPECT_TRUE(graph.are_nodes_adjacent(id2,id3));

    // Delete 4, then 5 becomes isolated, 6 and 7 are connected, 1, 3, and 2 are connected
    graph.delete_node(id4);
    EXPECT_TRUE(graph.is_node_isolated(id5));
    EXPECT_TRUE(graph.are_nodes_adjacent(id6,id7));
    EXPECT_TRUE(graph.are_nodes_adjacent(id1,id3));
    EXPECT_TRUE(graph.are_nodes_adjacent(id2,id3));

    // Try to access a non-existent vertex
    EXPECT_THROW(graph.delete_node(id4),std::invalid_argument);
    EXPECT_THROW(graph.connect_nodes(id1,id4),std::invalid_argument);
    EXPECT_THROW(graph.disconnect_nodes(id4,id6),std::invalid_argument);
}

TEST_F(SimpleGraphTestFixtureWithVertices, SimpleGraphTraversalAndReachableVerticesWorks)
{
    EXPECT_FALSE(graph.are_nodes_reachable(id0,id1));
    EXPECT_FALSE(graph.are_nodes_reachable(id0,id2));
    EXPECT_FALSE(graph.are_nodes_reachable(id0,id3));
    EXPECT_FALSE(graph.are_nodes_reachable(id0,id4));
    EXPECT_FALSE(graph.are_nodes_reachable(id0,id5));
    EXPECT_FALSE(graph.are_nodes_reachable(id0,id6));
    EXPECT_FALSE(graph.are_nodes_reachable(id0,id7));
    EXPECT_TRUE(graph.are_nodes_reachable(id1,id2));
    EXPECT_TRUE(graph.are_nodes_reachable(id1,id3));
    EXPECT_TRUE(graph.are_nodes_reachable(id1,id4));
    EXPECT_TRUE(graph.are_nodes_reachable(id1,id5));
    EXPECT_TRUE(graph.are_nodes_reachable(id1,id6));
    EXPECT_TRUE(graph.are_nodes_reachable(id1,id7));

    graph.delete_node(id4);
    EXPECT_TRUE(graph.is_node_isolated(id5));
    EXPECT_FALSE(graph.are_nodes_reachable(id0,id1));
    EXPECT_TRUE(graph.are_nodes_reachable(id1,id2));
    EXPECT_TRUE(graph.are_nodes_reachable(id1,id3));
    EXPECT_FALSE(graph.are_nodes_reachable(id1,id7));
    EXPECT_FALSE(graph.are_nodes_reachable(id2,id7));
    EXPECT_FALSE(graph.are_nodes_reachable(id3,id7));
    EXPECT_FALSE(graph.are_nodes_reachable(id5,id7));
    EXPECT_FALSE(graph.are_nodes_reachable(id1,id5));
    EXPECT_TRUE(graph.are_nodes_reachable(id6,id7));

    EXPECT_THAT(graph.get_reachable_nodes(id0),ElementsAre(id0));
    EXPECT_THAT(graph.get_reachable_nodes(id1),ElementsAre(id1,id2,id3));
    EXPECT_THAT(graph.get_reachable_nodes(id6),ElementsAre(id6,id7));
    EXPECT_THAT(graph.get_reachable_nodes(id5),ElementsAre(id5));
}


