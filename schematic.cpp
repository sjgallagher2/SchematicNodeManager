#include "schematic.h"
#include <set>

#include <iostream>
#include <cctype>  // ::isdigit

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
    // note that isolated() doesn't depend on traversal
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

template <typename T>
void print_Vec(const string& name, const Estd::Vec<T>& v)
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
    cout << "\t("<<w.first<<", "<<w.second<<")"<<endl;
}

void Schematic::print()
{
    // print spanning trees
    _update_trees();
    update_nets();
    for(auto& tr : _vtrees)
    {
        print_Vec("Tree: ",tr);
    }

    // print spanning trees by edges
    int i=0;
    for(auto& tr : _etrees)
    {
        cout << "Tree " << i << " ****************"<< endl;
        for(auto& w : tr) print_Wire(w);
        i++;
    }

    // print nets
    for(auto& pair : _nets)
    {
        cout << "******************\n";
        cout << "Net name: " << pair.first << endl;
        cout << "Wires: \n";
        for(auto& w : pair.second) print_Wire(w);
    }
}

bool netname_is_int(const string& name)
{
    return !name.empty() && std::all_of(name.begin(), name.end(), ::isdigit);
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
        map<string,Vec<Wire>> nets_new;
        std::set<int> ok_trees;
        std::set<string> ok_nets;
        for(int treeid=0; treeid < _etrees.size(); treeid++)
        {
            for(auto& nm : _nets)
            {
                if(nm.second == _etrees[treeid])
                {
                    nets_new[nm.first] = nm.second;
                    ok_trees.insert(treeid);
                    ok_nets.insert(nm.first);
                    break;
                }
            }
        }
        // Any nets not in `ok_nets` can be removed and (if integer)
        // added back to the id pool
        for(auto& pair : _nets)
        {
            if(ok_nets.find(pair.first) == ok_nets.end())
            {
                // See if name is a plain number, and if so, add back to pool
                // NOTE: This means we CANNOT allow users to set numerical net names
                if(netname_is_int(pair.first))
                {
                    int netnum = std::stoi(pair.first);
                    _idpool.put_back(netnum);
                }
            }
        }
        // Any trees not in `ok_trees` must be given net names
        for(int treeid=0; treeid < _etrees.size(); treeid++)
        {
            if(ok_trees.find(treeid) == ok_trees.end())
            {
                // Rename net
                int net_num = _idpool.get();
                string net_name = std::to_string(net_num);
                nets_new[net_name] = _etrees[treeid];
            }
        }

        _nets = nets_new;
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


