#include "simplegraph.h"
#include "utils.h"
#include <utility>
#include <algorithm>

using Estd::Vec;
using std::map;
using std::pair;
// using std::move;
using std::find;
using std::make_unique;
using Estd::find;
using Estd::find_if;
using Estd::contains;
using Estd::any_of;

using GraphNodeP = std::unique_ptr<GraphNode>;

Vec<int> SimpleGraph::get_all_node_ids()
{
    Vec<int> vids(_nodes.size());
    for(int i=0; i<_nodes.size();i++)
    {
        vids[i] = _nodes[i]->get_id();
    }
    return vids;
}

const GraphNode& SimpleGraph::get_node(int id)
{
    for(auto& v : _nodes)
    {
        if(v->get_id() == id) return *v;
    }
    throw std::invalid_argument("Supplied id is not in the graph.");
}

int SimpleGraph::add_node(bool traverse)
{
    GraphNodeP nn = make_unique<GraphNode>(_idpool.get());
    int nodeid = nn->get_id();

    // add to nodes
    _nodes.push_back(std::move(nn));

    // Create a new entry in `adjacent` with an empty list
    _adjacent.insert(pair(nodeid,Vec<int>{}));

    if(traverse) traverse_graph();

    return nodeid;
}

void SimpleGraph::add_node(GraphNodeP nn, bool traverse)
{
    // Protected method for adding nodes by pointer
    // usage: add_node(std::move(make_unique<NodeDerivedType>(idpool.get(),...)))
    int nodeid = nn->get_id();

    // add to nodes
    _nodes.push_back(std::move(nn));

    // Create a new entry in `adjacent` with an empty list
    _adjacent.insert(pair(nodeid,Vec<int>{}));

    if(traverse) traverse_graph();
}

int SimpleVertexGraph::add_vertex(Coordinate2 p, bool traverse)
{
    // First check if this position is already present
    {
        Vertex* tempv;
        for(auto& other : _nodes)
        {
            // Borrowing our unique_ptr just for this loop
            tempv = static_cast<Vertex*>(other.get());
            if(tempv->get_pos() == p)
            {
                return tempv->get_id();
            }
        }
    }

    // If we haven't returned, this is a new position
    int nodeid = _idpool.get();
    add_node(std::move(make_unique<Vertex>(nodeid,p)),traverse);
    return nodeid;
}

void SimpleGraph::connect_nodes(int id1,int id2, bool traverse)
{
    // Check if already connected
    // Performs check for id1 and id2 in graph, throws invalid_argument if not
    if(are_nodes_adjacent(id1,id2)) return;

    // Check if id1 == id2 (error)
    if(id1 == id2)
    {
        throw std::invalid_argument("Cannot connect a vertex to itself.");
    }

    // Connect
    _adjacent[id1].push_back(id2);
    _adjacent[id2].push_back(id1);

    if(traverse) traverse_graph();
}
void SimpleGraph::disconnect_nodes(int id1,int id2, bool traverse)
{
    // Check if disconnected
    // Performs check for id1 and id2 in graph, throws invalid_argument if not
    if(!are_nodes_adjacent(id1,id2)) return;

    // Check if id1 == id2 (do nothing)
    if(id1 == id2) return;

    // Disconnect
    // Update id1 list (if not found, do nothing)
    auto p1 = find(_adjacent[id1].begin(),_adjacent[id1].end(),id2);
    if(p1 != _adjacent[id1].end()) _adjacent[id1].erase(p1);
    // Update id2 list (if not found, do nothing)
    auto p2 = find(_adjacent[id2].begin(),_adjacent[id2].end(),id1);
    if(p2 != _adjacent[id2].end()) _adjacent[id2].erase(p2);

    if(traverse) traverse_graph();
}
void SimpleGraph::delete_node(int id, bool traverse)
{
    try {  // Checks that id is in graph, throws invalid_argument otherwise
        // For each vertex in the adjacency list `_adjacent[id]`, call disconnect_vertices()
        Vec<int> adj_id = _adjacent.at(id);  // Copy this so we don't modify while looping
        for(auto id_other : adj_id)
        {
            disconnect_nodes(id,id_other,false);
        }
    } catch (std::out_of_range) {
        throw std::invalid_argument("Supplied id is not in the graph.");
    }

    // Now that the vertex is isolated, we delete it from the list of vertices
    for(auto itr = _nodes.begin(); itr != _nodes.end(); ++itr)
    {
        if((*itr)->get_id() == id)
        {
            _nodes.erase(itr);
            break;
        }
    }

    // Remove the entry in `adjacent` for this id
    _adjacent.erase(id);

    // Retraverse if asked for
    if(traverse) traverse_graph();

    // Add vertex id back to the id pool
    _idpool.put_back(id);
}



/* For testing only  */
#include <iostream>
#include <string>
using std::cout;
using std::endl;
using std::string;
template <typename T>
void print_Vec(const string&& name, const Vec<T>& v)
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

bool SimpleGraph::are_nodes_reachable(int id1, int id2, bool force_traverse)
{
    if(force_traverse) traverse_graph();  // Update vertex_tree_id
    if(_node_tree_id.empty()) traverse_graph();
    try {
        return _node_tree_id.at(id1) == _node_tree_id.at(id2);
    } catch (std::out_of_range) {
        throw std::invalid_argument("One of the supplied ids is not in the graph.");
    }
}

void SimpleGraph::traverse_graph()
{
    /* Set Up */
    using CItr = Vec<GraphNodeP>::const_iterator;  // Iterators are safe, underlying Vec won't change

    Vec<int> parents;            // Stack of parents to return to
    map<CItr,int> vert_dfs_id;   // New ids in traversal order
    int dfs_id = 0;              // running id value, gets incremented
    int tree_id = -1;             // Tree id for new spanning trees
    _node_tree_id.clear();      // remap vertex id -> tree id

    // Make bimap for id <-> iterator
    map<CItr,int> vert_itr_id;   // iterator -> id
    map<int,CItr> vert_id_itr;   // id -> iterator
    map<int,bool> visited;       // Visited nodes map
    for(auto vitr = _nodes.begin(); vitr != _nodes.end(); ++vitr)
    {
        vert_itr_id[vitr] = (*vitr)->get_id();
        vert_id_itr[(*vitr)->get_id()] = vitr;
        visited[(*vitr)->get_id()] = false;
    }

    /*
     * Graph traversal by depth-first search
     * Loop over all _nodes
     * Mark the first vertex as visited, check if it has any adjacent _nodes
     *   If adjacents, check if they are visited
     *      For any unvisited adjacents, set them as the next vertex to visit, push current vertex into parent
     *   If no more adjacents, pop parents
     *      If no more parents, increment until we find an unvisited vertex
     */
    CItr current_itr = _nodes.cbegin();
    CItr next_itr = _nodes.cend();
    int current_id = 0;              // Current id, for convenience
    while(current_itr != _nodes.cend())
    {
        // 1. Initialize
        current_id = vert_itr_id[current_itr];
        next_itr = _nodes.cend();
        Vec<int> current_adjs = _adjacent[current_id];

        // 2. Check: Is `parents` empty and all nodes are visited?
        if(parents.empty())
        {
            // Check if all true (w/ lambda)
            if(Estd::all_of(visited, [](const pair<int,bool>& r){return r.second;}))
            {
                current_itr = _nodes.cend();
                continue;
            }

            // 2.b. If no parents and unvisited, start a new tree
            if(!visited[current_id])
            {
                tree_id++;  // This marks the start of a new tree
                // cout << "-------------------\n";
                // cout << "Starting new tree " << tree_id << endl;
            }
        }

        // 3. Mark vertex as visited if not already
        if(!visited[current_id])
        {
            visited[current_id] = true;
            vert_dfs_id[current_itr]=dfs_id;
            dfs_id++;
            _node_tree_id[current_id] = tree_id;
        }

        /* Optional: Print stuff */
        // cout << "Id = " << current_id << endl;
        // print_Vec("parents",parents);
        // print_Vec("current_adjs",current_adjs);
        // print_map("visited",visited);
        /***************/

        // 4. Check for unvisited adjacent nodes
        for(auto vadj_id : current_adjs)
        {
            if(!visited[vadj_id])
            {
                // Unvisited adjacent vertex
                // Set as the next
                parents.push_back(current_id);
                next_itr = vert_id_itr[vadj_id];
                break; // break out of for loop
            }
        }
        // 5. Check for a parent, and if one is found, pop it
        if(next_itr == _nodes.end())
        {
            // All (if any) adjacent nodes are visited
            // Check for parent, pop
            if(parents.size() > 0)
            {
                next_itr = vert_id_itr[parents.back()];
                parents.pop_back();
            }
        }
        // 6. If no adjacent nodes or parent, increment the iterator
        if(next_itr == _nodes.end())
        {
            next_itr = std::next(current_itr);
        }

        // 7. Update current itr
        current_itr = next_itr;
    }

    // cout << "Map from old to new ids after traversal:\n";
    // for(auto vitr = vertices.cbegin(); vitr != vertices.cend(); ++vitr)
    // {
    //     cout << vitr->get_id() << "  -->  " << vert_dfs_id[vitr];
    //     cout << "\t\ttree " << vertex_tree_id[vitr->get_id()] << endl;
    // }

}




bool SimpleGraph::are_nodes_adjacent(int id1,int id2)
{
    // Check if id2 is in the graph (check for id1 is implicit
    if(!(any_of(_nodes,MatchingId{id2}))) { throw std::invalid_argument("Supplied id2 is not in the graph."); }

    try
    {
        Vec<int>& id1_adj = _adjacent.at(id1);
        return find(id1_adj.begin(),id1_adj.end(),id2) != id1_adj.end();
    }
    catch(std::out_of_range)
    {
        throw std::invalid_argument("Supplied id1 is not in the graph.");
    }
}
bool SimpleGraph::is_node_isolated(int id)
{
    try
    {
        Vec<int>& id_adj = _adjacent.at(id);
        return id_adj.size() == 0;
    }
    catch(std::out_of_range){ throw std::invalid_argument("Supplied id is not in the graph."); }
}
Vec<int> SimpleGraph::get_reachable_nodes(int id, bool force_traverse)
{
    if(force_traverse) traverse_graph();
    // Check if id exists
    if(!(any_of(_nodes,MatchingId{id}))) { throw std::invalid_argument("Supplied id is not in the graph."); }

    int tree_id = -1;
    try{
        tree_id = _node_tree_id.at(id);
    }catch(std::out_of_range){
        throw std::invalid_argument("Could not find vertex id in tree map, traversal might be stale.");
    }

    // Select all vertices matching that tree id
    Vec<int> reachables;
    for(auto& pair : _node_tree_id)
    {
        if(pair.second == tree_id) reachables.push_back(pair.first);
    }
    return reachables;
}



