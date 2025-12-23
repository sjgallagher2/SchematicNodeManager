#include "simplegraph.h"
#include <utility>
#include <algorithm>

using std::vector;
using std::map;
using std::pair;
using std::find;

vector<int> SimpleGraph::get_vertex_ids()
{
    vector<int> vids(vertices.size());
    for(int i=0; i<vertices.size();i++)
    {
        vids[i] = vertices[i].get_id();
    }
    return vids;
}

Coordinate2 SimpleGraph::get_vertex_pos(int id)
{
    // Check if id exists
    if(!_has_id(id))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }
    Vertex& v = get_vertex(id);
    return v.get_pos();
}

Vertex& SimpleGraph::get_vertex(int id)
{
    // Check if id exists
    if(!_has_id(id))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }
    for(auto& v : vertices)
    {
        if(v.get_id() == id) return v;
    }
    throw std::runtime_error("Id should be in the graph, but was not found.");
}

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
    if(are_vertices_adjacent(id1,id2)) return;

    // Check if id1 == id2 (error)
    if(id1 == id2)
    {
        throw std::invalid_argument("Cannot connect a vertex to itself.");
    }

    // Connect
    adjacent[id1].push_back(id2);
    adjacent[id2].push_back(id1);
}
void SimpleGraph::disconnect_vertices(int id1,int id2)
{
    // Check if these are valid ids
    if((!_has_id(id1)) || (!_has_id(id2)))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }

    // Check if disconnected
    if(!are_vertices_adjacent(id1,id2)) return;

    // Check if id1 == id2 (do nothing)
    if(id1 == id2) return;

    // Disconnect
    // Update id1 list (if not found, do nothing)
    auto p1 = find(adjacent[id1].begin(),adjacent[id1].end(),id2);
    if(p1 != adjacent[id1].end()) adjacent[id1].erase(p1);
    // Update id2 list (if not found, do nothing)
    auto p2 = find(adjacent[id2].begin(),adjacent[id2].end(),id1);
    if(p2 != adjacent[id2].end()) adjacent[id2].erase(p2);
}
void SimpleGraph::delete_vertex(int id)
{
    // Check if id exists
    if(!_has_id(id))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }

    // For each vertex in the adjacency list `adjacent[id]`, call disconnect_vertices()
    vector<int> adj_id = adjacent[id];  // Copy this so we don't modify while looping
    for(auto id_other : adj_id)
    {
        disconnect_vertices(id,id_other);
    }

    // Now that the vertex is isolated, we delete it from the list of vertices
    for(auto itr = vertices.begin(); itr != vertices.end(); ++itr)
    {
        if(itr->get_id() == id)
        {
            vertices.erase(itr);
            break;
        }
    }

    // Finally, remove the entry in `adjacent` for this id
    adjacent.erase(id);

    // Add vertex id back to the id pool (TODO)
}
bool SimpleGraph::are_vertices_adjacent(int id1,int id2)
{
    // Check if these are valid ids
    if((!_has_id(id1)) || (!_has_id(id2)))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }

    vector<int>& id1_adj = adjacent[id1];
    return find(id1_adj.begin(),id1_adj.end(),id2) != id1_adj.end();
}
bool SimpleGraph::is_vertex_isolated(int id)
{
    // Check if id exists
    if(!_has_id(id))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }

    vector<int>& id_adj = adjacent[id];
    return id_adj.size() == 0;
}
const vector<int>& SimpleGraph::get_reachable_vertices(int id)
{
    // Check if id exists
    if(!_has_id(id))
    {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }
    // placeholder, only return immediately adjacent vertices
    return adjacent[id];
}

void SimpleGraph::traverse_graph()
{
    /*
     * Depth-first search graph traversal.
     * Each vertex is visited if it hasn't been visited already.
     * For each vertex visited, visit each of its adjacent vertices.
     * If no more vertices, go back to predecessor.
     */


    // To do this, start with an empty `visited` vector, and an int:int map
    // for id:dfs_id. Initialize the map values for each id to all zeros.
    // Keep a value `dfs_id` which will be incremented for each visited vertex.
    // To start the traversal, take the first vertex. Mark it as visited and
    // assign its dfs_id, then increment dfs_id.
    // To continue the traversal:
    //   - If:  there are no adjacent vertices, continue to next by moving linearly
    //     through the list of vertices and checking for unvisited nodes
    //   - Else if there are adjacent vertices, check for any that are unvisited.
    //   --- If none are unvisited, check for a parent and pop
    //   ------- If no parent, continue traversal
    //

    vector<int> visited;  // Empty vector, visited vertices are pushed back
    map<int,int> vert_dfs_ids;  // ids for depth-first search, default to 0
    vector<int> parents;  // Stack for parent ids

    // Now for efficiency, make a map from id to Vertex iterator (iterators will stay
    // valid since we're not changing the tree)
    vector<int> all_ids = get_vertex_ids();
    map<int,vector<Vertex>::const_iterator> vertex_lookup;

    for(auto& id : all_ids) vert_dfs_ids[id] = 0;
    for(auto vitr = vertices.begin(); vitr != vertices.end(); ++vitr)
    {
        vertex_lookup[vitr->get_id()] = vitr;
    }
    int dfs_id = 1;  // static id to be assigned, incremented after each visit


}

bool SimpleGraph::_has_id(int id)
{
    for(auto& other : vertices)
    {
        if(other.get_id() == id) return true;
    }
    return false;
}


