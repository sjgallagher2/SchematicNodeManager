#include "simplegraph.h"
#include <utility>
#include <algorithm>

using std::vector;
using std::map;
using std::pair;
using std::find;

int SimpleGraph::add_vertex(Coordinate2 p)
{
    Vertex vn(_next_available_index, p);
    // First check if this vertex is already present
    for(auto& other : vertices)
    {
        if(other == vn) return other.get_id();
    }
    // Vertex is new, so add to vertices
    vertices.push_back(vn);
    _next_available_index++; // Increment next available always (for now)

    // Create a new entry in `adjacent` with an empty list
    adjacent.insert(pair(vn.get_id(),vector<int>{}));
    return vn.get_id();
}
void SimpleGraph::connect_vertices(int id1,int id2)
{
    // Check if these are valid ids
    if((!_has_id(id1)) || (!_has_id(id2)))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }

    // Check if already connected
    if(are_vertices_connected(id1,id2)) return;

    // Connect
    adjacent[id1].push_back(id2);
    adjacent[id2].push_back(id1);
}
bool SimpleGraph::are_vertices_connected(int id1,int id2)
{
    // Check if these are valid ids
    if((!_has_id(id1)) || (!_has_id(id2)))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }

    vector<int> id1_adj = adjacent[id1];
    return find(id1_adj.begin(),id1_adj.end(),id2) != id1_adj.end();
}
bool SimpleGraph::is_vertex_isolated(int id)
{
    // Check if id exists
    if(!_has_id(id))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }

    return false;
}
vector<int> SimpleGraph::get_reachable_vertices(int id)
{
    // Check if id exists
    if(!_has_id(id))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }
    return adjacent[id];
}

bool SimpleGraph::_has_id(int id)
{
    for(auto& other : vertices)
    {
        if(other.get_id() == id) return true;
    }
    return false;
}


