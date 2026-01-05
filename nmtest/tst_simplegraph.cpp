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

/* For testing only  */
#include <iostream>
#include <string>
using std::cout;
using std::endl;
using std::string;
template <typename T>
void print_Vec(const string&& name, const Estd::Vec<T>& v)
{
    if(v.size() == 0) cout << name << " = []\n";
    else
    {
        cout << name << " = [";
        for(int i=0; i< v.size()-1; i++) cout << v[i] << ", ";
        cout << v.back() << "]\n";
    }
}
template<typename T,typename V>
void print_map(const string&& name, const map<T,V> mm)
{
    if(mm.size() == 0) cout << name << " = []\n";
    else
    {
        cout << std::boolalpha;
        cout << name << " = [\n";
        for(auto const& pair : mm)
        {
            cout << "\t" << pair.first << " : " << pair.second << endl;
        }
        cout << std::noboolalpha;
    }
}
/* /For testing only  */





class SimpleGraphTestFixture : public Test
{
protected:
    SimpleGraph graph;
    // Default constructor
};

class VertexGraphTestFixture : public Test
{
protected:
    VertexGraph graph;
};

class SimpleGraphTestFixtureWithNodes : public Test
{
protected:
    SimpleGraph graph;
    SimpleGraphTestFixtureWithNodes() {
        id0 = graph.add(false);
        id1 = graph.add(false);
        id2 = graph.add(false);
        id3 = graph.add(false);
        id4 = graph.add(false);
        id5 = graph.add(false);
        id6 = graph.add(false);
        id7 = graph.add(false);

        graph.connect(id1,id3,false);
        graph.connect(id2,id3,false);
        graph.connect(id3,id4,false);
        graph.connect(id4,id5,false);
        graph.connect(id4,id6,false);
        graph.connect(id4,id7,false);
        graph.connect(id6,id7,true);  // with traversal
    }
    int id0,id1,id2,id3,id4,id5,id6,id7;
};


class VertexGraphTestFixtureWithVertices : public Test
{
protected:
    VertexGraph graph;
    VertexGraphTestFixtureWithVertices() {
        id0 = graph.add({ 0, 0},false);
        id1 = graph.add({ 2, 1},false);
        id2 = graph.add({ 6, 2},false);
        id3 = graph.add({ 7,-1},false);
        id4 = graph.add({ 2,-2},false);
        id5 = graph.add({ 0,-4},false);
        id6 = graph.add({-4,-5},false);
        id7 = graph.add({-1,-6},false);
        id8 = graph.add({ 2,-6},false);

        graph.connect(id1,id2,false);
        graph.connect(id1,id4,false);
        graph.connect(id2,id3,false);
        graph.connect(id3,id4,false);
        graph.connect(id4,id5,false);
        graph.connect(id5,id6,false);
        graph.connect(id5,id7,false);
        graph.connect(id5,id8,true);
    }
    int id0,id1,id2,id3,id4,id5,id6,id7,id8;
};

class VertexGraphTestFixtureWithDegenerates : public Test
{
protected:
    VertexGraph graph;
    VertexGraphTestFixtureWithDegenerates() {
        id0  = graph.add({ 0, 0},false);
        id1  = graph.add({ 0, 1},false);  // degen
        id2  = graph.add({ 0, 2},false);  // degen
        id3  = graph.add({ 0, 3},false);
        id4  = graph.add({ 1, 3},false);  // degen
        id5  = graph.add({ 2, 3},false);  // degen
        id6  = graph.add({ 3, 3},false);
        id7  = graph.add({ 4, 3},false);
        id8  = graph.add({ 3, 2},false);
        id9  = graph.add({ 3, 0},false);
        id10 = graph.add({ 4, 0},false);
        id11 = graph.add({ 5, 0},false);  // degen
        id12 = graph.add({ 6, 0},false);  // degen
        id13 = graph.add({ 7, 0},false);  // degen
        id14 = graph.add({ 8, 0},false);
        id15 = graph.add({ 9, 0},false);

        graph.connect(id0 ,id1 ,false);
        graph.connect(id1 ,id2 ,false);
        graph.connect(id2 ,id3 ,false);
        graph.connect(id3 ,id4 ,false);
        graph.connect(id4 ,id5 ,false);
        graph.connect(id5 ,id6 ,false);
        graph.connect(id6 ,id7 ,false);
        graph.connect(id6 ,id8 ,false);
        graph.connect(id8 ,id9 ,false);
        graph.connect(id8 ,id10,false);
        graph.connect(id9 ,id10,false);
        graph.connect(id10,id11,false);
        graph.connect(id11,id12,false);
        graph.connect(id12,id13,false);
        graph.connect(id13,id14,true );
    }
    int id0,id1,id2,id3,id4,id5,id6,id7,id8,id9,id10,id11,id12,id13,id14,id15;
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
    int id = graph.add();
    EXPECT_EQ(0,id);
    id = graph.add();
    EXPECT_EQ(1,id);
    id = graph.add();
    EXPECT_EQ(2,id);
    vector<int> adj_vtxs = graph.get_reachable(id);
    EXPECT_THAT(adj_vtxs,ElementsAre(id));
}

TEST_F(VertexGraphTestFixture, SimpleVertexGraphAddVertexWorks)
{
    int id = graph.add(Coordinate2(0,0));
    EXPECT_EQ(0,id);
    id = graph.add(Coordinate2(0,0));
    EXPECT_EQ(0,id);  // Doesn't add anything
    id = graph.add(Coordinate2(0,5));
    EXPECT_EQ(1,id);
    vector<int> adj_vtxs = graph.get_reachable(id);
    EXPECT_THAT(adj_vtxs,ElementsAre(id));
}

TEST_F(SimpleGraphTestFixture, SimpleGraphInitializesTwoVerticesCorrectly)
{
    // Can we construct nodes
    int id1 = graph.add();
    int id2 = graph.add();

    /* Were the vertices added properly */
    // Are these vertices connected (they should not be)
    // Throw invalid_argument if either id does not exist
    EXPECT_THROW(graph.adjacent(99,0),std::invalid_argument);
    EXPECT_THROW(graph.adjacent(0,99),std::invalid_argument);
    EXPECT_FALSE(graph.adjacent(id1,id2));
    // Are these vertices both isolated (they should be)
    EXPECT_TRUE(graph.isolated(id1));
    EXPECT_TRUE(graph.isolated(id2));
    // Calling is_vertex_isolated on a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.isolated(99),std::invalid_argument);
    // Are these the only vertex id's listed (they should be)
    vector<int> vids = graph.get_all_ids();
    EXPECT_THAT(vids,ElementsAre(id1,id2));
}

TEST_F(VertexGraphTestFixture, SimpleVertexGraphInitializesTwoVerticesCorrectly)
{
    // Can we construct vertices
    Coordinate2 p1(0,0);
    Coordinate2 p2(0,5);
    int id1 = graph.add(p1);
    int id2 = graph.add(p2);

    /* Were the vertices added properly */
    // Are these vertices connected (they should not be)
    // Throw invalid_argument if either id does not exist
    EXPECT_THROW(graph.adjacent(99,0),std::invalid_argument);
    EXPECT_THROW(graph.adjacent(0,99),std::invalid_argument);
    EXPECT_FALSE(graph.adjacent(id1,id2));
    // Are these vertices both isolated (they should be)
    EXPECT_TRUE(graph.isolated(id1));
    EXPECT_TRUE(graph.isolated(id2));
    // Calling is_vertex_isolated on a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.isolated(99),std::invalid_argument);
    // Are these the only vertex id's listed (they should be)
    vector<int> vids = graph.get_all_ids();
    EXPECT_THAT(vids,ElementsAre(id1,id2));
}

TEST_F(SimpleGraphTestFixture, SimpleGraphConnectTwoVerticesWorks)
{
    int id1 = graph.add();
    int id2 = graph.add();

    /* Testing connections */
    // Can we connect to a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.connect(id1,99),std::invalid_argument);
    EXPECT_THROW(graph.connect(99,id1),std::invalid_argument);
    // Can we connect two existing vertices (they should become connected)
    graph.connect(id1,id2);
    EXPECT_TRUE(graph.adjacent(id1,id2));
    EXPECT_TRUE(graph.adjacent(id2,id1));
    // Are these vertices both isolated (they should not be anymore)
    EXPECT_FALSE(graph.isolated(id1));
    EXPECT_FALSE(graph.isolated(id2));
    // Is it safe to call connect_vertices() twice (it should be)
    graph.connect(id2,id1);
    EXPECT_TRUE(graph.adjacent(id1,id2));
    EXPECT_TRUE(graph.adjacent(id2,id1));
    // Can we call connect_vertices() with the same vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.connect(id1,id1),std::invalid_argument);
    EXPECT_THROW(graph.connect(id2,id2),std::invalid_argument);
}

TEST_F(SimpleGraphTestFixture, SimpleGraphDisconnectVerticesWorks)
{
    int id1 = graph.add();
    int id2 = graph.add();
    graph.connect(id1,id2);

    /* Testing disconnections */
    // Can we disconnect a non-existent vertex (should fail with invalid_argument)
    EXPECT_THROW(graph.disconnect(id1,99),std::invalid_argument);
    EXPECT_THROW(graph.disconnect(99,id1),std::invalid_argument);
    // Are these vertices both isolated (they should not be)
    EXPECT_FALSE(graph.isolated(id1));
    EXPECT_FALSE(graph.isolated(id2));
    // Can we disconnect two existing vertices (they should become isolated)
    graph.disconnect(id1,id2);
    EXPECT_FALSE(graph.adjacent(id1,id2));
    EXPECT_FALSE(graph.adjacent(id2,id1));
    EXPECT_TRUE(graph.isolated(id1));
    EXPECT_TRUE(graph.isolated(id2));
    // Is it safe to call disconnect_vertices() on disconnected vertices  (it should be)
    graph.disconnect(id2,id1);
    EXPECT_FALSE(graph.adjacent(id1,id2));
    EXPECT_FALSE(graph.adjacent(id2,id1));
    // Can we call disconnect_vertices() with the same vertex (should do nothing)
    EXPECT_NO_THROW(graph.disconnect(id1,id1));
    EXPECT_NO_THROW(graph.disconnect(id2,id2));
}

TEST_F(SimpleGraphTestFixtureWithNodes, SimpleGraphDeleteVerticesWorks)
{
    // Verify some connections
    EXPECT_FALSE(graph.isolated(id5));
    EXPECT_TRUE(graph.adjacent(id6,id7));
    EXPECT_TRUE(graph.adjacent(id1,id3));
    EXPECT_TRUE(graph.adjacent(id2,id3));

    // Delete 4, then 5 becomes isolated, 6 and 7 are connected, 1, 3, and 2 are connected
    graph.erase(id4);
    EXPECT_TRUE(graph.isolated(id5));
    EXPECT_TRUE(graph.adjacent(id6,id7));
    EXPECT_TRUE(graph.adjacent(id1,id3));
    EXPECT_TRUE(graph.adjacent(id2,id3));

    // Try to access a non-existent vertex
    EXPECT_THROW(graph.erase(id4),std::invalid_argument);
    EXPECT_THROW(graph.connect(id1,id4),std::invalid_argument);
    EXPECT_THROW(graph.disconnect(id4,id6),std::invalid_argument);
}

TEST_F(SimpleGraphTestFixtureWithNodes, SimpleGraphTraversalAndReachableVerticesWorks)
{
    EXPECT_FALSE(graph.reachable(id0,id1));
    EXPECT_FALSE(graph.reachable(id0,id2));
    EXPECT_FALSE(graph.reachable(id0,id3));
    EXPECT_FALSE(graph.reachable(id0,id4));
    EXPECT_FALSE(graph.reachable(id0,id5));
    EXPECT_FALSE(graph.reachable(id0,id6));
    EXPECT_FALSE(graph.reachable(id0,id7));
    EXPECT_TRUE(graph.reachable(id1,id2));
    EXPECT_TRUE(graph.reachable(id1,id3));
    EXPECT_TRUE(graph.reachable(id1,id4));
    EXPECT_TRUE(graph.reachable(id1,id5));
    EXPECT_TRUE(graph.reachable(id1,id6));
    EXPECT_TRUE(graph.reachable(id1,id7));

    graph.erase(id4);
    EXPECT_TRUE(graph.isolated(id5));
    EXPECT_FALSE(graph.reachable(id0,id1));
    EXPECT_TRUE(graph.reachable(id1,id2));
    EXPECT_TRUE(graph.reachable(id1,id3));
    EXPECT_FALSE(graph.reachable(id1,id7));
    EXPECT_FALSE(graph.reachable(id2,id7));
    EXPECT_FALSE(graph.reachable(id3,id7));
    EXPECT_FALSE(graph.reachable(id5,id7));
    EXPECT_FALSE(graph.reachable(id1,id5));
    EXPECT_TRUE(graph.reachable(id6,id7));

    EXPECT_THAT(graph.get_reachable(id0),ElementsAre(id0));
    EXPECT_THAT(graph.get_reachable(id1),ElementsAre(id1,id2,id3));
    EXPECT_THAT(graph.get_reachable(id6),ElementsAre(id6,id7));
    EXPECT_THAT(graph.get_reachable(id5),ElementsAre(id5));
}

TEST_F(SimpleGraphTestFixtureWithNodes, SimpleGraphGetEdgeListReturnsCorrect)
{
    Estd::Vec<pair<int,int>> expected;
    expected.push_back(pair<int,int>(1,3));
    expected.push_back(pair<int,int>(2,3));
    expected.push_back(pair<int,int>(3,4));
    expected.push_back(pair<int,int>(4,5));
    expected.push_back(pair<int,int>(4,6));
    expected.push_back(pair<int,int>(4,7));
    expected.push_back(pair<int,int>(6,7));
    EXPECT_EQ(graph.get_all_edges(),expected);

    expected.clear();
    expected.push_back(pair<int,int>(1,3));
    expected.push_back(pair<int,int>(2,3));
    expected.push_back(pair<int,int>(6,7));
    graph.erase(id4);
    EXPECT_EQ(graph.get_all_edges(),expected);
}

TEST_F(SimpleGraphTestFixtureWithNodes, SimpleGraphSpanningTreesAreCorrect)
{
    using Estd::Vec;
    Vec<Vec<int>> expected;
    expected.push_back({id0});
    expected.push_back({id1,id3,id2,id4,id5,id6,id7});
    EXPECT_EQ(expected,graph.get_spanning_trees());

    graph.erase(id4);

    expected.clear();
    expected.push_back({id0});
    expected.push_back({id1,id3,id2});
    expected.push_back({id5});
    expected.push_back({id6,id7});
    EXPECT_EQ(expected,graph.get_spanning_trees());
}

TEST_F(SimpleGraphTestFixtureWithNodes, SimpleGraphGetSubgraphReturnsStatic)
{
    Estd::Vec<int> subnodes {1,2,3};
    auto sub1_adj = graph.get_sub_adjacency_lists(subnodes);
    SimpleStaticGraph sub1(subnodes,sub1_adj);
    EXPECT_TRUE(sub1.reachable(id1,id2));
    EXPECT_TRUE(sub1.reachable(id1,id3));
    EXPECT_TRUE(sub1.reachable(id2,id3));
    EXPECT_THROW(sub1.reachable(id1,id4),std::invalid_argument);
}

TEST_F(VertexGraphTestFixtureWithVertices, VertexGraphAddNodeOnEdgeSplitsEdge)
{
    // Adding a point not on an edge has no effect on the edge connections
    EXPECT_TRUE(graph.adjacent(id4,id5));
    int id9 = graph.add({1.,-2.});
    EXPECT_TRUE(graph.adjacent(id4,id5));
    EXPECT_FALSE(graph.adjacent(id9,id4));
    EXPECT_FALSE(graph.adjacent(id9,id5));

    // Adding a point at (1,-3) splits the edge between 4 and 5
    EXPECT_TRUE(graph.adjacent(id4,id5));
    int id10 = graph.add({1.,-3.});
    EXPECT_FALSE(graph.adjacent(id4,id5));  // not adjacent but reachable
    EXPECT_TRUE(graph.reachable(id4,id5));
    EXPECT_TRUE(graph.adjacent(id10,id4));
    EXPECT_TRUE(graph.adjacent(id10,id5));
}

TEST_F(VertexGraphTestFixtureWithVertices, VertexGraphConnectNodesCollinearAndNoncollinear)
{
    // New positions are (-1,-5) and (3,-1)
    // These are collinear with id2, id4, id5
    int id9 = graph.add({-1,-5});
    int id10 = graph.add({3,-1});

    // Try connecting id9 to id5
    // No intermediate points
    graph.connect(id9,id5);
    EXPECT_TRUE(graph.reachable(id9,id2));
    EXPECT_FALSE(graph.reachable(id9,id10));

    // Now connect id9 to id10
    // id4 and id5 are intermediates
    graph.connect(id9,id10);
    EXPECT_TRUE(graph.reachable(id9,id10));
    EXPECT_TRUE(graph.reachable(id9,id2));
    EXPECT_TRUE(graph.adjacent(id9,id5));
    EXPECT_TRUE(graph.adjacent(id5,id4));
    EXPECT_TRUE(graph.adjacent(id4,id10));
    EXPECT_FALSE(graph.adjacent(id9,id10));
    EXPECT_FALSE(graph.adjacent(id4,id2));

    graph.disconnect(id9,id5);
    graph.disconnect(id10,id4);
    EXPECT_TRUE(graph.isolated(id9));
    EXPECT_TRUE(graph.isolated(id10));

    // Finally remove id10, connect id9 to id2
    graph.erase(id10);
    graph.connect(id9,id2);
    EXPECT_TRUE(graph.reachable(id9,id2));
    EXPECT_TRUE(graph.adjacent(id9,id5));
    EXPECT_TRUE(graph.adjacent(id5,id4));
    EXPECT_TRUE(graph.adjacent(id4,id2));
}

TEST_F(VertexGraphTestFixtureWithDegenerates,VertexGraphMergeCollinearNodes)
{
    // Calling merge_unbranched_collinear_edges should remove id1, id2, id4, id5,
    // id11, id12, and id13
    graph.merge_unbranched_collinear_edges();

    Estd::Vec<int> allids = graph.get_all_ids();
    EXPECT_THAT(allids,Contains(id0));
    EXPECT_THAT(allids,Contains(id3));
    EXPECT_THAT(allids,Contains(id6));
    EXPECT_THAT(allids,Contains(id7));
    EXPECT_THAT(allids,Contains(id8));
    EXPECT_THAT(allids,Contains(id9));
    EXPECT_THAT(allids,Contains(id10));
    EXPECT_THAT(allids,Contains(id14));

    EXPECT_THAT(allids,Not(Contains(id1)));
    EXPECT_THAT(allids,Not(Contains(id2)));
    EXPECT_THAT(allids,Not(Contains(id4)));
    EXPECT_THAT(allids,Not(Contains(id5)));
    EXPECT_THAT(allids,Not(Contains(id11)));
    EXPECT_THAT(allids,Not(Contains(id12)));
    EXPECT_THAT(allids,Not(Contains(id13)));

    // Verify new connections
    EXPECT_TRUE(graph.adjacent(id0,id3));
    EXPECT_TRUE(graph.adjacent(id3,id6));
    EXPECT_TRUE(graph.adjacent(id10,id14));
}
