#include "schematic.h"
#include <set>

#include <iostream>
#include <cctype>  // ::isdigit

using std::cout;
using std::endl;

using std::map;
using std::multimap;
using std::string;
using std::set;
using Estd::Vec;
using std::find;
using Estd::find;
using Estd::find_if;
using Estd::contains;
using Estd::any_of;

using Wire = std::pair<int,int>;

// Initialize static data
const Wire Schematic::INVALID_WIRE{-1,-1};
// /static

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
void print_Wire(const Wire& w, const Coordinate2& a, const Coordinate2& b)
{
    cout << "\t{"<<w.first<<", "<<w.second<<"}\t = \t";
    cout << "{("<<a.x<<","<<a.y<<"), ("<<b.x<<","<<b.y<<")}\n";
}
void print_Wire(const Wire& w)
{
    cout << "\t{"<<w.first<<", "<<w.second<<"}\n";
}



Vec<string> Schematic::get_all_netnames()
{
    Vec<string> names;
    for(auto& pair : _nets) names.push_back(pair.first);
    return names;
}

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
    // First check if this wire would be degenerate
    Wire wdeg = Schematic::INVALID_WIRE;
    WireType degen = _degenerate(a,b,wdeg);
    if(degen == WireType::WIRE_DEGENERATE) {return Schematic::INVALID_WIRE;}

    // Now do the normal adding procedure
    int id1 = _graph.add(a,false);
    int id2 = _graph.add(b,false);
    _graph.connect(id1,id2,false);
    if(traverse)
    {
        _remove_degenerate_wires();  // This is a one-and-done method for updating.
        // Handles degenerate wires, and calls update_nets(), which in turn
        // calls _update_trees().
    }

    return {id1,id2};
}

/* Check if wire with coords (a,b) would be degenerate. Set `deg` to existing wire.
 *
 */
WireType Schematic::_degenerate(Coordinate2 a,Coordinate2 b,Wire& deg)
{
    if(a == b) return WireType::WIRE_DEGENERATE;
    Wire w1 = select_wire(a);
    Wire w2 = select_wire(b);
    if(w1 != Schematic::INVALID_WIRE)
    {
        // Same wire selected, this could be degenerate
        // (or a and b are the endpoints, in which case return the end points)
        if(w1 == w2)
        {
            Coordinate2 wp1 = _graph.pos(w1.first);
            Coordinate2 wp2 = _graph.pos(w1.second);
            if(wp1 == a && wp2 == b) return WireType::WIRE_NORMAL;
            if(wp1 == b && wp2 == a) return WireType::WIRE_NORMAL;
            deg = w1;
            return WireType::WIRE_DEGENERATE;
        }
    }

    return WireType::WIRE_NORMAL;
}

/* Remove any degenerate wires (collinear non-branching wires).
 * Always calls update_nets().
 */
void Schematic::_remove_degenerate_wires()
{
    _graph.merge_unbranched_collinear_edges();
    update_nets();
}

string Schematic::get_netname(Wire w)
{
    for(auto& nm : _nets)
    {
        // nm = name map
        for(auto& wtmp : nm.second)
        {
            if(wtmp == w) return nm.first;
            Wire w2(w.second,w.first);  // check for flipped coords
            if(wtmp == w2) return nm.first;
        }
    }
    throw std::invalid_argument("Wire is not associated with a net.");
}

Vec<Wire> Schematic::select_net(string netname)
{
    // _nets is a multimap, so collect all the trees for this netname
    // if none, throw invalid_argument

    Vec<Wire> selected;
    auto[range_start,range_end] = _nets.equal_range(netname);
    if(range_start == _nets.end()) {throw std::invalid_argument("Net name was not found in schematic.");}

    for(auto& itr = range_start; itr != range_end; ++itr)
    {
        for(auto w : itr->second)
        {
            selected.push_back(w);
        }
    }
    return selected;
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
    return Schematic::INVALID_WIRE;
}

/* Select every wire which overlaps `p`. If none, return empty Vec.
 */
Vec<Wire> Schematic::select_wires(Coordinate2 p)
{
    Vec<Wire> edges = get_all_wires();
    Vec<Wire> selected;
    double tol = p.prec();
    double dist = 0;
    for(auto& e : edges)
    {
        dist = distance_from_line(p, _graph.pos(e.first),_graph.pos(e.second));
        if(dist < tol) selected.push_back(e);
    }
    return selected;
}

bool Schematic::remove_wire(Wire w, bool traverse)
{
    _graph.disconnect(w.first,w.second,false);
    // note that isolated() doesn't depend on traversal
    if(_graph.isolated(w.first)) _graph.erase(w.first,false);
    if(_graph.isolated(w.second)) _graph.erase(w.second,false);

    if(traverse) update_nets();

    return true;
}

bool netname_is_int(const string& name)
{
    return !name.empty() && std::all_of(name.begin(), name.end(), ::isdigit);
}

/* Update net names.
 * Reprocess the whole schematic, basic heuristics for new net names. Calls _update_trees().
 * Call this when changes to the schematic are made. Methods like add_wire() and
 * remove_wire() will call it automatically if traverse==true (default).
 */
void Schematic::update_nets()
{
    // Note: This is a big function, but breaking it up would be uglier imho.

    // Go through current spanning trees, compare with spanning trees in _nets
    // Only add _nets keys that correspond to existing spanning trees in _etrees.
    // If any spanning tree is not a value in _nets, give it a new name, checking
    // for ports
    multimap<string,Vec<Wire>> nets_new;
    std::set<int> ok_trees;
    std::set<string> ok_nets;
    _update_trees();

    int treeid = -1; // init treeid
    // For each net (name : etree), search _etrees for it
    for(auto& net : _nets)
    {
        // lower_bound returns iter to first elem !< val, so
        // verify that it actually matches
        auto itr_lb = std::lower_bound(_etrees.begin(),_etrees.end(),net.second);
        if(itr_lb != _etrees.end() && *itr_lb == net.second)
        {
            // if match, get index of tree
            treeid = std::distance(_etrees.begin(),itr_lb);
            // update
            ok_trees.insert(treeid);
            nets_new.insert(net);
            ok_nets.insert(net.first);
        }
    }

    // For each net, check if net tree includes any tree in _etrees
    // Only search through trees that are not in ok_trees
    for(auto& net : _nets)
    {
        if(ok_nets.find(net.first) == ok_nets.end())
        {
            for(treeid=0; treeid < _etrees.size(); treeid++)
            {
                if(ok_trees.find(treeid) == ok_trees.end())
                {
                    // tree and net have not been placed yet
                    if(std::includes(net.second.begin(),net.second.end(),
                                      _etrees[treeid].begin(),_etrees[treeid].end()))
                    {
                        // _etrees[treeid] is a subset of net.second
                        // Wire has been removed, tree may have been split
                        // Since this net is not in ok_nets, this is the first
                        // subset of this net to appear. Add it with new subset.
                        ok_trees.insert(treeid);
                        net.second = _etrees[treeid];
                        nets_new.insert(net);
                        ok_nets.insert(net.first);
                    }
                    else if(std::includes(_etrees[treeid].begin(),_etrees[treeid].end(),
                            net.second.begin(),net.second.end()))
                    {
                        // net.second is a subset of _etrees[treeid]
                        // Wire has been added, two (or more?) trees may have been connected
                        // We can add this safely, after updating
                        ok_trees.insert(treeid);
                        net.second = _etrees[treeid];
                        nets_new.insert(net);
                        ok_nets.insert(net.first);
                    }
                }
            }
        }
    }

    // Any nets not in `ok_nets` are not added back;
    // if integer, names are added back to the id pool
    for(auto& pair : _nets)
    {
        if(ok_nets.find(pair.first) == ok_nets.end())
        {
            // See if name is a plain number, and if so, add back to pool
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
            // First check for ports
            bool has_port = false;
            for(auto& p : _ports)
            {
                // get first matching wire
                Wire port_wire = select_wire(p.first);
                if(port_wire != Schematic::INVALID_WIRE) {
                    // Check if this wire is in the current tree
                    if(contains(_etrees[treeid],port_wire)) {
                        // Port is connected to this tree
                        nets_new.insert({p.second, _etrees[treeid]});
                        has_port = true;
                        break;  // stop looking for ports
                    }
                }
            }

            if(!has_port)
            {
                // Otherwise, use default name
                int net_num = _idpool.get();
                string net_name = std::to_string(net_num);
                nets_new.insert({net_name, _etrees[treeid]});
            }
        }
    }

    _nets = nets_new;
}

/* Add a new port node.
 * The port name cannot be an integer.
 * Ports must have unique positions, but they do NOT need unique names (e.g. GND).
 * Port names are case-insensitive.
 */
int Schematic::add_port_node(Port port, bool traverse)
{
    // Check name
    if(netname_is_int(port.second)) {return -1;}
    // Check for duplicate ports
    for(int i=0; i<_ports.size(); i++)
    {
        if(_ports[i].first == port.first) { return i; }
    }

    // Add port
    Estd::to_lower(port.second);  // by reference
    _ports.push_back(port);

    // Remove any trees from _nets to force refactor
    Wire pw = select_wire(port.first);
    if(pw != Schematic::INVALID_WIRE)
    {
        // This port overlaps a wire
        // Remove its tree from _nets
        try{
            string netname = get_netname(pw);
            if(_nets.find(netname) != _nets.end())
            {
                // Return netname to pool if integer
                if(netname_is_int(netname))
                {
                    int netnum = std::stoi(netname);
                    _idpool.put_back(netnum);
                }
                auto[range_start,range_end] = _nets.equal_range(netname);
                _nets.erase(range_start,range_end);
            }
        } catch(std::invalid_argument){}
    }

    if(traverse) update_nets();
    return _ports.size()-1;  // last element
}

/*
 * Return the index of a port sufficiently close to `p`, or -1 if none found.
 */
int Schematic::select_port_node(Coordinate2 p) const
{
    for(int i=0; i<_ports.size(); i++)
    {
        if(_ports[i].first == p)
        {
            return i;
        }
    }
    return -1;
}

/*
 * Return indices of all ports with the name `port_name`, or empty Vec if none.
 */
Vec<int> Schematic::select_port_nodes(std::string port_name) const
{
    Vec<int> selected;
    for(int i=0; i<_ports.size(); i++)
    {
        if(_ports[i].second == port_name) {selected.push_back(i);}
    }
    return selected;
}

void Schematic::remove_port_node(int pid, bool traverse)
{
    try{
        // erase port from `_ports`
        _ports.erase(_ports.begin()+pid);
        // remove entries in `_nets`
        // Note: this is aggressive, but update_nets() will rename any that
        // still have this name
        string netname = _ports[pid].second;
        auto[range_start,range_end] = _nets.equal_range(netname);
        _nets.erase(range_start,range_end);
        if(traverse) {update_nets();}
    }catch(std::out_of_range){
        throw std::invalid_argument("Port node not found in Schematic.");
    }
}

/*
 * Remove all port nodes with name `port_name`. Do nothing if none found.
 */
void Schematic::remove_port_nodes(std::string port_name, bool traverse)
{
    // Remove any ports with this name from _ports
    Vec<Port> new_ports;
    for(auto& p : _ports)
    {
        if(p.second != port_name) {new_ports.push_back(p);}
    }
    _ports = new_ports;

    // Remove entries in _nets
    auto[range_start,range_end] = _nets.equal_range(port_name);
    _nets.erase(range_start,range_end);
    if(traverse) {update_nets();}
}

void Schematic::print()
{
    update_nets();

    // print nets
    for(auto& pair : _nets)
    {
        cout << "******************\n";
        cout << "Net name: " << pair.first << endl;
        cout << "Wires: \n";
        Coordinate2 a,b;
        for(auto& w : pair.second)
        {
            a = _graph.pos(w.first);
            b = _graph.pos(w.second);
            print_Wire(w,a,b);
        }
    }
}

/*
 * Update the spanning trees of vertices.
 * This method clears _vtrees and _etrees and repopulates them. The _nets data
 * structure is NOT UPDATED by this method. It is called by update_nets().
 * Each edge tree in _etrees is sorted, and then _etrees itself is sorted by first
 * element.
 */
void Schematic::_update_trees()
{
    _etrees.clear();
    for(auto& tree : _graph.get_spanning_trees(true))
    {
        auto sub_adj = _graph.get_sub_adjacency_lists(tree);
        SimpleStaticGraph subgraph(tree,sub_adj);
        _etrees.push_back(subgraph.get_all_edges());
        std::sort(_etrees.back().begin(),_etrees.back().end());  // sort lexicographically
    }
    std::sort(_etrees.begin(),_etrees.end());
}






