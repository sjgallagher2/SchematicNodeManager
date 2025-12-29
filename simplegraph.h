#ifndef SIMPLEGRAPH_H
#define SIMPLEGRAPH_H

#include <map>
#include <queue>
#include <memory>
#include "utils.h"
#include "coordinate2.h"

using Estd::Vec;
using std::map;
using std::pair;
using std::queue;
using std::unique_ptr;


class GraphNode
{
private:
    int _id;
public:
    GraphNode(int id): _id{id} {}
    int get_id() const {return _id;}
};

// Vertex has a position and id. The id cannot be changed. Make a new one if needed.
class Vertex : public GraphNode
{
private:
    Coordinate2 _p;
public:
    Vertex(int id, Coordinate2 p): GraphNode{id},_p{p} {}
    Coordinate2 get_pos() const {return _p;}
    void set_pos(Coordinate2 p) {_p = p;}
};
// Vertex comparison operator is based on position, NOT id
inline bool operator==(const Vertex& lhs, const Vertex& rhs)
{
    return lhs.get_pos() == rhs.get_pos();
}


/*
 * Basic integer pool for ids in a graph.
 * Return an id to the pool with put_back()
 */
class IdPool
{
public:
    IdPool() : _pool_size{1} {_free_ids.push(0);};
    inline int get() {
        if(_free_ids.empty())
        {
            _free_ids.push(_pool_size);
            _pool_size++;
        }
        int next_id = _free_ids.front();
        _free_ids.pop();
        return next_id;
    }
    inline void put_back(int id)
    {
        if((id >= 0)&&(id < _pool_size)) _free_ids.push(id);
        else throw std::out_of_range("Id returned to pool was not from pool originally.");
    }
private:
    queue<int> _free_ids;
    int _pool_size;
};


/* SimpleGraph manages a collection of Vertices and their connections in an
 * adjacency list.
 *
 * You can add and delete vertices, connect vertices together, disconnect them,
 * and check if one vertex is reachable from another. Connecting a vertex to
 * itself is invalid and will raise std::invalid_argument. Vertices are identified
 * by an int `id` which must be stored. You can get a list of vertices by id, or
 * look up a vertex by its position.
 *
 * Internally, each id has a vector of adjacent vertices (an adjacency list) which
 * is updated to reflect the current state of the graph.
 */

class SimpleGraph
{
public:
    using GraphNodeP = unique_ptr<GraphNode>;

    SimpleGraph() {}
    virtual ~SimpleGraph() {}
    Vec<int> get_all_node_ids();

    // Modify graph, optional traversal afterwards
    // Only set traverse=false if you have a large number of
    // operations or a large graph, and if you plan to traverse
    // later.
    int add_node(bool traverse=true);
    void connect_nodes(int,int,bool traverse=true);
    void disconnect_nodes(int,int,bool traverse=true);
    void delete_node(int,bool traverse=true);

    // Get info, no traversal required
    bool are_nodes_adjacent(int,int);
    bool is_node_isolated(int);

    // Get info, traversal required
    bool are_nodes_reachable(int id1,int id2,bool force_traverse=false);
    Vec<int> get_reachable_nodes(int,bool force_traverse=false);

    // Graph traversal, depth-first search
    void traverse_graph();

protected:
    void add_node(GraphNodeP nn,bool traverse=true);
    IdPool _idpool;
    Vec<GraphNodeP> _nodes;            // Node vector

private:
    map<int,Vec<int>> _adjacent;       // Adjacent vertices of each node by id
    const GraphNode& get_node(int);   // Get node by id
    map<int,int> _node_tree_id;        // Map of node id -> tree id

};



class SimpleVertexGraph : public SimpleGraph
{
public:
    using GraphNodeP = unique_ptr<GraphNode>;
    SimpleVertexGraph() {}
    virtual ~SimpleVertexGraph() {}

    int add_vertex(Coordinate2,bool traverse=true);
};

// Predicate for matching ids
struct MatchingId{
    int val;
    MatchingId(int v) : val{v}{}
    bool operator()(const unique_ptr<GraphNode>& r){return r->get_id()==val;}
};


#endif // SIMPLEGRAPH_H
