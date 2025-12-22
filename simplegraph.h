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



/* SimpleGraph manages a collection of Vertices and their connections in an
 * adjacency list.
 *
 * You can add and remove vertices, connect vertices together, disconnect them,
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
    SimpleGraph() : _next_available_index{0} {}
    vector<int> get_vertex_ids();
    Coordinate2 get_vertex_pos(int);
    int add_vertex(Coordinate2 p);
    void connect_vertices(int,int);
    void disconnect_vertices(int,int);
    void delete_vertex(int);
    bool are_vertices_adjacent(int,int);
    bool is_vertex_isolated(int);
    const vector<int>& get_reachable_vertices(int);
    int find_vertex(Coordinate2 p);

private:
    vector<Vertex> vertices;  // Vertex index in vector
    map<int,vector<int>> adjacent;  // Adjacent vertices of each vertex id
    int _next_available_index;
    bool _has_id(int id);
    Vertex& get_vertex(int);  // Get vertex by id
};

#endif // SIMPLEGRAPH_H
