#include "schematic.h"
#include <set>

#include <iostream>

using std::cout;
using std::endl;

using std::map;
using std::string;
using std::set;
using Estd::Vec;
using std::find;
using Estd::find;
using Estd::find_if;
using Estd::contains;
using Estd::any_of;

using Wire = std::pair<int,int>;

/*
 * Add a wire to the schematic.
 * This method returns a Wire (a pair of vertex indices), however we only guarantee that
 * the two vertices are _reachable_, not adjacent. That means you should _NOT_ pass
 * the returned Wire to methods such as get_netname(). It's arguable whether this
 * method should return anything at all.
 *
 * Use `traverse=false` if you are adding many wires and don't want to re-traverse
 * every time.
 */
Wire Schematic::add_wire(Coordinate2 a, Coordinate2 b, bool traverse)
{
    int id1 = _graph.add(a,false);
    int id2 = _graph.add(b,false);
    _graph.connect(id1,id2,false);
    if(traverse) _update_trees();  // calls traverse_graph()
    return {id1,id2};
}

string Schematic::get_netname(Wire w)
{
    for(auto& nm : _nets)
    {
        // nm = name map
        for(auto& wtmp : nm.second)
        {
            if(wtmp == w) return nm.first;
        }
    }
    throw std::invalid_argument("Wire is not associated with a net.");
}

Vec<Wire> Schematic::select_net(string netname)
{
    try {
        return _nets.at(netname);
    } catch (std::out_of_range) {
        throw std::invalid_argument("No net with given name.");
    }
}

/* Return a Wire sufficiently close to point `p`, or (-1,-1) if none found.
 * Uses p.prec() to determine tolerance for distance to Wire. Returns the first
 * Wire found.
 */
Wire Schematic::select_wire(Coordinate2 p)
{
    Vec<Wire> edges = get_all_wires();
    double tol = p.prec();
    double dist = 0;
    for(auto& e : edges)
    {
        dist = distance_from_line(p, _graph.pos(e.first),_graph.pos(e.second));
        if(dist < tol) return e;
    }
    return {-1,-1};
}

bool Schematic::remove_wire(Wire w, bool traverse)
{
    _graph.disconnect(w.first,w.second,false);
    // isolated() doesn't depend on traversal
    if(_graph.isolated(w.first)) _graph.erase(w.first,false);
    if(_graph.isolated(w.second)) _graph.erase(w.second,false);

    if(traverse) _update_trees();

    return true;
}

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
void print_Wire(const Wire& w)
{
    cout << "("<<w.first<<", "<<w.second<<")"<<endl;
}

void Schematic::print()
{
    // for now, print spanning trees
    _update_trees();
    for(auto& tr : _vtrees)
    {
        print_Vec("Tree: ",tr);
    }

    int i=0;
    for(auto& tr : _etrees)
    {
        cout << "Tree " << i << " ****************"<< endl;
        for(auto& w : tr) print_Wire(w);
        i++;
    }
}

/* Update net names.
 * If vhint1 (only) or both vhint1 and vhint2 are supplied, only update nets
 * that involve those vertices. This is much more efficient.
 * If no hints are provided, reprocess the whole schematic, no heuristics
 * for new net names.
 */
void Schematic::update_nets(int vhint1, int vhint2)
{
    if(vhint1 == -1 && vhint2 == -1)
    {
        // Go through current spanning trees, compare with spanning trees in _nets
        // Remove _nets keys that don't correspond to existing spanning trees
        // If any spanning tree is not a value in _nets, give it a new name, checking
        // for ports
        for(auto& tree : _vtrees)
        {
            for(auto& nm : _nets)
            {
                //
            }
        }
    }
}

/*
 * Update the spanning trees of vertices.
 * This method clears _vtrees and repopulates it. The _nets data structure
 * is NOT UPDATED by this method.
 */
void Schematic::_update_trees()
{
    _vtrees = _graph.get_spanning_trees(true);

    _etrees.clear();
    for(auto& tree : _vtrees)
    {
        auto sub_adj = _graph.get_sub_adjacency_lists(tree);
        SimpleStaticGraph subgraph(tree,sub_adj);
        _etrees.push_back(subgraph.get_all_edges());
    }
}


