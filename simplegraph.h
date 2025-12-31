#ifndef SIMPLEGRAPH_H
#define SIMPLEGRAPH_H

#include <map>
#include <queue>
#include <memory>
#include <type_traits>
#include <utility>
#include <algorithm>
#include "utils.h"
#include "coordinate2.h"


class GraphNode
{
private:
    int _id;
public:
    GraphNode(int id): _id{id} {}
    int get_id() const {return _id;}
};

// GraphVertex has a position and id. The id cannot be changed. Make a new one if needed.
class GraphVertex : public GraphNode
{
private:
    Coordinate2 _p;
public:
    GraphVertex(int id, Coordinate2 p): GraphNode{id},_p{p} {}
    Coordinate2 get_pos() const {return _p;}
    void set_pos(Coordinate2 p) {_p = p;}
};
// GraphVertex comparison operator is based on position, NOT id
inline bool operator==(const GraphVertex& lhs, const GraphVertex& rhs)
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
    std::queue<int> _free_ids;
    int _pool_size;
};


/* AbstractGraph manages a collection of Nodes and their connections in an
 * adjacency list.
 *
 * You can add and delete nodes, connect nodes together, disconnect them,
 * and check if one node is reachable from another. Connecting a node to
 * itself is invalid and will raise std::invalid_argument. Nodes are identified
 * by an int `id` which must be stored. You can get a list of nodes by id, or
 * look up a nodeby its position.
 *
 * Internally, each id has a Estd::Vector of adjacent nodes (an adjacency list) which
 * is updated to reflect the current state of the graph.
 */
template<typename NodeT>
class AbstractGraph
{
public:
    static_assert(std::is_base_of<GraphNode,NodeT>::value,"NodeT must derive from Graphnode");
    using GraphNodeP = std::unique_ptr<NodeT>;

    AbstractGraph() {}
    virtual ~AbstractGraph() {}
    Estd::Vec<int> get_all_ids()
    {
        Estd::Vec<int> vids(_nodes.size());
        for(int i=0; i<_nodes.size();i++)
        {
            vids[i] = _nodes[i]->get_id();
        }
        return vids;
    }

    // Predicate for matching ids
    struct MatchingId{
        int val;
        MatchingId(int v) : val{v}{}
        bool operator()(const std::unique_ptr<NodeT>& r){return r->get_id()==val;}
    };

    //virtual int add(bool traverse=true)
    virtual void connect(int id1,int id2,bool traverse=true) {_connect_nodes(id1,id2,traverse);}
    virtual void disconnect(int id1,int id2,bool traverse=true) {_disconnect_nodes(id1,id2,traverse);}
    virtual void erase(int id,bool traverse=true) {_delete_node(id,traverse);}

    // Get info, no traversal required
    virtual bool adjacent(int id1,int id2) {return _are_nodes_adjacent(id1,id2);}
    virtual bool isolated(int id) {return _is_node_isolated(id);}

    // Get info, traversal required
    virtual bool reachable(int id1,int id2,bool force_traverse=false) {return _are_nodes_reachable(id1,id2,force_traverse);}
    virtual Estd::Vec<int> get_reachable(int id,bool force_traverse=false) {return _get_reachable_nodes(id,force_traverse);}

    // Graph traversal, depth-first search
    virtual void traverse_graph() {_traverse_graph();}
    virtual Estd::Vec<std::pair<int,int>> get_edge_list() {return _get_edge_list();}
    const Estd::Vec<int>& get_adjacent(int id)
    {
        try{
            return _adjacent.at(id);
        }catch (std::out_of_range)
        {
            throw std::invalid_argument("Supplied id1 is not in the graph.");
        }
    }

protected:
    /*********************************/
    // Modify graph, optional traversal afterwards
    // Only set traverse=false if you have a large number of
    // operations or a large graph, and if you plan to traverse
    // later.
    int _add_node(bool traverse)
    {
        GraphNodeP nn = std::make_unique<NodeT>(_idpool.get());
        int nodeid = nn->get_id();

        // add to nodes
        _nodes.push_back(std::move(nn));

        // Create a new entry in `adjacent` with an empty list
        _adjacent.insert(std::pair(nodeid,Estd::Vec<int>{}));

        if(traverse) _traverse_graph();

        return nodeid;
    }

    void _add_node(GraphNodeP nn, bool traverse)
    {
        // Protected method for adding nodes by pointer
        // usage: add_node(std::move(std::make_unique<NodeDerivedType>(idpool.get(),...)))
        int nodeid = nn->get_id();

        // add to nodes
        _nodes.push_back(std::move(nn));

        // Create a new entry in `adjacent` with an empty list
        _adjacent.insert(std::pair(nodeid,Estd::Vec<int>{}));

        if(traverse) _traverse_graph();
    }

    void _connect_nodes(int id1,int id2, bool traverse)
    {
        // Check if already connected
        // Performs check for id1 and id2 in graph, throws invalid_argument if not
        if(_are_nodes_adjacent(id1,id2)) return;

        // Check if id1 == id2 (error)
        if(id1 == id2)
        {
            throw std::invalid_argument("Cannot connect a node to itself.");
        }

        // Connect
        _adjacent[id1].push_back(id2);
        _adjacent[id2].push_back(id1);

        if(traverse) _traverse_graph();
    }
    void _disconnect_nodes(int id1,int id2, bool traverse)
    {
        // Check if disconnected
        // Performs check for id1 and id2 in graph, throws invalid_argument if not
        if(!_are_nodes_adjacent(id1,id2)) return;

        // Check if id1 == id2 (do nothing)
        if(id1 == id2) return;

        // Disconnect
        // Update id1 list (if not found, do nothing)
        auto p1 = find(_adjacent[id1].begin(),_adjacent[id1].end(),id2);
        if(p1 != _adjacent[id1].end()) _adjacent[id1].erase(p1);
        // Update id2 list (if not found, do nothing)
        auto p2 = find(_adjacent[id2].begin(),_adjacent[id2].end(),id1);
        if(p2 != _adjacent[id2].end()) _adjacent[id2].erase(p2);

        if(traverse) _traverse_graph();
    }
    void _delete_node(int id, bool traverse)
    {
        try {  // Checks that id is in graph, throws invalid_argument otherwise
            // For each node in the adjacency list `_adjacent[id]`, call disconnect_vertices()
            Estd::Vec<int> adj_id = _adjacent.at(id);  // Copy this so we don't modify while looping
            for(auto id_other : adj_id)
            {
                _disconnect_nodes(id,id_other,false);
            }
        } catch (std::out_of_range) {
            throw std::invalid_argument("Supplied id is not in the graph.");
        }

        // Now that the node is isolated, we delete it from the list of vertices
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
        if(traverse) _traverse_graph();

        // Add node id back to the id pool
        _idpool.put_back(id);
    }

    bool _are_nodes_reachable(int id1, int id2, bool force_traverse)
    {
        if(force_traverse) _traverse_graph();  // Update node_tree_id
        if(_node_tree_id.empty()) _traverse_graph();
        try {
            return _node_tree_id.at(id1) == _node_tree_id.at(id2);
        } catch (std::out_of_range) {
            throw std::invalid_argument("One of the supplied ids is not in the graph.");
        }
    }

    void _traverse_graph()
    {
        /* Set Up */
        using CItr = typename Estd::Vec<GraphNodeP>::const_iterator;  // Iterators are safe, underlying Estd::Vec won't change

        Estd::Vec<int> parents;            // Stack of parents to return to
        std::map<CItr,int> vert_dfs_id;   // New ids in traversal order
        int dfs_id = 0;              // running id value, gets incremented
        int tree_id = -1;             // Tree id for new spanning trees
        _node_tree_id.clear();      // remap node id -> tree id

        // Make bimap for id <-> iterator
        std::map<CItr,int> vert_itr_id;   // iterator -> id
        std::map<int,CItr> vert_id_itr;   // id -> iterator
        std::map<int,bool> visited;       // Visited nodes map
        for(auto vitr = _nodes.begin(); vitr != _nodes.end(); ++vitr)
        {
            vert_itr_id[vitr] = (*vitr)->get_id();
            vert_id_itr[(*vitr)->get_id()] = vitr;
            visited[(*vitr)->get_id()] = false;
        }

        /*
         * Graph traversal by depth-first search
         * Loop over all _nodes
         * Mark the first node as visited, check if it has any adjacent _nodes
         *   If adjacents, check if they are visited
         *      For any unvisited adjacents, set them as the next node to visit, push current node into parent
         *   If no more adjacents, pop parents
         *      If no more parents, increment until we find an unvisited node
         */
        CItr current_itr = _nodes.cbegin();
        CItr next_itr = _nodes.cend();
        int current_id = 0;              // Current id, for convenience
        while(current_itr != _nodes.cend())
        {
            // 1. Initialize
            current_id = vert_itr_id[current_itr];
            next_itr = _nodes.cend();
            Estd::Vec<int> current_adjs = _adjacent[current_id];

            // 2. Check: Is `parents` empty and all nodes are visited?
            if(parents.empty())
            {
                // Check if all true (w/ lambda)
                if(Estd::all_of(visited, [](const std::pair<int,bool>& r){return r.second;}))
                {
                    current_itr = _nodes.cend();
                    continue;
                }

                // 2.b. If no parents and unvisited, start a new tree
                if(!visited[current_id])
                {
                    tree_id++;  // This marks the start of a new tree
                }
            }

            // 3. Mark node as visited if not already
            if(!visited[current_id])
            {
                visited[current_id] = true;
                vert_dfs_id[current_itr]=dfs_id;
                dfs_id++;
                _node_tree_id[current_id] = tree_id;
            }

            // 4. Check for unvisited adjacent nodes
            for(auto vadj_id : current_adjs)
            {
                if(!visited[vadj_id])
                {
                    // Unvisited adjacent node
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
        //     cout << "\t\ttree " << node_tree_id[vitr->get_id()] << endl;
        // }

    }

    /*********************************/
    // Get info, no traversal required
    bool _are_nodes_adjacent(int id1,int id2)
    {
        // Check if id2 is in the graph (check for id1 is implicit
        if(!(any_of(_nodes,MatchingId{id2}))) { throw std::invalid_argument("Supplied id2 is not in the graph."); }

        try
        {
            Estd::Vec<int>& id1_adj = _adjacent.at(id1);
            return find(id1_adj.begin(),id1_adj.end(),id2) != id1_adj.end();
        }
        catch(std::out_of_range)
        {
            throw std::invalid_argument("Supplied id1 is not in the graph.");
        }
    }
    bool _is_node_isolated(int id)
    {
        try
        {
            Estd::Vec<int>& id_adj = _adjacent.at(id);
            return id_adj.size() == 0;
        }
        catch(std::out_of_range){ throw std::invalid_argument("Supplied id is not in the graph."); }
    }
    Estd::Vec<int> _get_reachable_nodes(int id, bool force_traverse)
    {
        if(force_traverse) _traverse_graph();
        // Check if id exists
        if(!(any_of(_nodes,MatchingId{id}))) { throw std::invalid_argument("Supplied id is not in the graph."); }

        int tree_id = -1;
        try{
            tree_id = _node_tree_id.at(id);
        }catch(std::out_of_range){
            throw std::invalid_argument("Could not find node id in tree map, traversal might be stale.");
        }

        // Select all vertices matching that tree id
        Estd::Vec<int> reachables;
        for(auto& pair : _node_tree_id)
        {
            if(pair.second == tree_id) reachables.push_back(pair.first);
        }
        return reachables;
    }

    const NodeT& _get_node(int id)
    {
        for(auto& v : _nodes)
        {
            if(v->get_id() == id) return *v;
        }
        throw std::invalid_argument("Supplied id is not in the graph.");
    }

    // Get Estd::Vector of edges as (id1,id2)
    Estd::Vec<std::pair<int,int>> _get_edge_list()
    {
        Estd::Vec<std::pair<int,int>> edges;
        // Go through adjacency lists
        // If node id > this, add (this,other) to edges
        for(auto& adjlist : _adjacent)
        {
            for(auto& oth : adjlist.second)
            {
                if(oth > adjlist.first) edges.push_back(std::pair<int,int>(adjlist.first,oth));
            }
        }
        return edges;
    }

    IdPool _idpool;                    // Id pool  -- only protected for add() methods
    Estd::Vec<GraphNodeP> _nodes;            // Node Estd::Vector

private:
    std::map<int,Estd::Vec<int>> _adjacent;       // Adjacent vertices of each node by id
    std::map<int,int> _node_tree_id;        // Map of node id -> tree id
};

/*
 * SimpleGraph is a minimal implementation of AbstractGraph
 * It exposes the graph methods to a public interface
 */
class SimpleGraph : public AbstractGraph<GraphNode>
{
public:
    virtual int add(bool traverse=true) {return _add_node(traverse);}
};

/*
 * VertexGraph implements AbstractGraph and provides additional
 * topological structure within Euclidean space.
 *
 * Basically a SimpleGraph but with nodes having coordinates (vertices)
 * and obeying the two rules below.
 *   1. Two vertices cannot share the same position
 *   2. Two edges cannot be both collinear _and_ overlapping
 */
class VertexGraph : public AbstractGraph<GraphVertex>
{
public:
    using VertexP = std::unique_ptr<GraphVertex>;
    using Edge = std::pair<int,int>;
    VertexGraph() {}
    virtual ~VertexGraph() {}

    /*
     * Add a vertex to a vertex graph.
     * Cases:
     *   - New vertex is in same location (within tol) of existing vertex
     *      => return existing vertex id
     *   - New vertex is on an existing edge (within tol)
     *      => disconnect existing vertices, add new vertex, connect both existing
     *         vertices to new vertex instead
     */
    int add(Coordinate2 p, bool traverse=true)
    {
        // First check if this position is already present
        for(auto& other : _nodes)
        {
            if(other->get_pos() == p)
            {
                return other->get_id();
            }
        }

        // If we haven't returned, we can safely add this Vertex
        int nodeid = _idpool.get();
        // This clunky syntax ensures we only add GraphVertex objects
        _add_node(std::move(std::make_unique<GraphVertex>(nodeid,p)),traverse);

        // Now check if this new vertex is on an existing edge
        Estd::Vec<Edge> edges = _get_edge_list();
        for(auto& edge : edges)
        {
            if(_on_edge(nodeid,edge))
            {
                _disconnect_nodes(edge.first,edge.second,false);
                _connect_nodes(nodeid,edge.first,false);
                _connect_nodes(nodeid,edge.second,true);
                break;  // ignore case when node lies on intersection of multiple edges
            }
        }

        return nodeid;
    }
    virtual void connect(int id1,int id2,bool traverse=true)
    {
        if(adjacent(id1,id2)) return;

        // Connect vertices, but only if the new edge would not be collinear with another
        // To do this, check if (id1,id2,idx) are collinear, where idx is any other
        // vertex.
        // If they are, mark that point.
        // Process the set of all collinear points so that id1 and id2 are reachable and
        // connect to their respective nearest collinear point.
        Coordinate2 p1 = _get_node(id1).get_pos();
        Coordinate2 p2 = _get_node(id2).get_pos();
        double tol = p1.prec();

        Estd::Vec<GraphVertex> collinear_vtxs;
        Estd::Vec<Coordinate2> collinear_coords;
        for(auto& other : _nodes)
        {
            // collinear() is pretty light, run it on all vertices
            int oth_id = other->get_id();
            if(oth_id != id1 && oth_id != id2)
            {
                if(collinear(p1,p2,other->get_pos(),tol))
                {
                    // Check if point is between p1 and p2
                    double dp = p1.distance(p2);
                    bool cond1 = p1.distance(other->get_pos()) < dp;
                    bool cond2 = p2.distance(other->get_pos()) < dp;
                    if(cond1 && cond2)
                    {
                        collinear_vtxs.push_back(*other); // copy it out
                        collinear_coords.push_back(other->get_pos());  // redundant but convenient
                    }
                }
            }
        }

        // If no collinear points in graph, simple connection
        if(collinear_vtxs.empty()) _connect_nodes(id1,id2,false);

        // Otherwise connect all collinear points in order of distance
        // Small optimization: using plain (x^2 + y^2) instead of sqrt(x^2+y^2) to sort
        // Start by adding the input points to the list
        collinear_vtxs.push_back(_get_node(id1));
        collinear_coords.push_back(p1);
        collinear_vtxs.push_back(_get_node(id2));
        collinear_coords.push_back(p2);
        // Now calculate "distances" from p1
        Estd::Vec<double> dists;
        for(auto& c : collinear_coords)
        {
            dists.push_back(std::pow(p1.x-c.x,2)+std::pow(p1.y-c.y,2));
        }
        // Get indices of sorted distances
        std::vector<size_t> sorted_idxs = Estd::argsort(dists);
        // Call _connect_nodes() on each adjacent pair of indices
        for(int i=0; i<sorted_idxs.size()-1; i++)
        {
            int tmp_id1 = collinear_vtxs[sorted_idxs[i]].get_id();
            int tmp_id2 = collinear_vtxs[sorted_idxs[i+1]].get_id();
            _connect_nodes(tmp_id1,tmp_id2,false);
        }
        // Note that no non-adjacent nodes can be connected, since they're handled
        // either during add() or during a previous call to connect()

        // Finish up
        if(traverse) _traverse_graph();
    }
    virtual void disconnect(int id1,int id2,bool traverse=true)
    {
        _disconnect_nodes(id1,id2,traverse);
    }
    virtual void erase(int id,bool traverse=true)
    {
        _delete_node(id,traverse);
    }

private:
    bool _on_edge(int id, Edge edge)
    {
        Coordinate2 p1 = _get_node(id).get_pos();
        Coordinate2 p2 = _get_node(edge.first).get_pos();
        Coordinate2 p3 = _get_node(edge.second).get_pos();
        double tol = p1.prec();
        if(distance_from_line(p1,p2,p3) < tol) return true;
        return false;
    }

};




#endif // SIMPLEGRAPH_H
