#ifndef SIMPLEGRAPH_H
#define SIMPLEGRAPH_H

#include <vector>
#include <map>
#include "coordinate2.h"

using std::vector;
using std::map;

// Vertex has a position and id. The id cannot be changed.
class Vertex
{
private:
    Coordinate2 _p;
    int _id;
public:
    Vertex(int id, Coordinate2 p): _p{p}, _id{id} {}
    Coordinate2 get_pos() const {return _p;}
    void set_pos(Coordinate2 p) {_p = p;}
    int get_id() const {return _id;}
};
// Vertex comparison operator is based on position, NOT id
inline bool operator==(const Vertex& lhs, const Vertex& rhs)
{
    return lhs.get_pos() == rhs.get_pos();
}



// SimpleGraph manages a collection of Vertices and their connections in an
// adjacency list
class SimpleGraph
{
public:
    SimpleGraph() : _next_available_index{0} {}
    int add_vertex(Coordinate2 p);
    void connect_vertices(int,int);
    bool are_vertices_connected(int,int);
    bool is_vertex_isolated(int);
    vector<int> get_reachable_vertices(int);

private:
    vector<Vertex> vertices;  // Vertex index in vector
    map<int,vector<int>> adjacent;  // Adjacent vertices of each vertex id
    int _next_available_index;
    bool _has_id(int id);
};

#endif // SIMPLEGRAPH_H
