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

// Initialize static data
const Wire Schematic::INVALID_WIRE{-1,-1};
// /static

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
    else if(degen == WireType::WIRE_PARTIAL_DEGEN_A)
    {
        // `a` was on the wire, replace it with endpoint farthest from `b`
        Coordinate2 wp1 = _graph.pos(wdeg.first);
        Coordinate2 wp2 = _graph.pos(wdeg.second);
        double dist1 = b.distance(wp1);
        double dist2 = b.distance(wp2);
        if(dist1 > dist2) {a = wp1;}
        else if(dist1 < dist2) {a = wp2;}
        else {throw std::logic_error("External point is equidistant from collinear endpoints! This shouldn't happen.");}
        // Remove existing wire, we'll replace it
        remove_wire(wdeg,false);
    }
    else if(degen == WireType::WIRE_PARTIAL_DEGEN_B)
    {
        // `b` was on the wire, replace it with endpoint farthest from `a`
        Coordinate2 wp1 = _graph.pos(wdeg.first);
        Coordinate2 wp2 = _graph.pos(wdeg.second);
        double dist1 = a.distance(wp1);
        double dist2 = a.distance(wp2);
        if(dist1 > dist2) {b = wp1;}
        else if(dist1 < dist2) {b = wp2;}
        else {throw std::logic_error("External point is equidistant from collinear endpoints! This shouldn't happen.");}
        // Remove existing wire, we'll replace it
        remove_wire(wdeg,false);
    }

    // Now do the normal adding procedure
    int id1 = _graph.add(a,false);
    int id2 = _graph.add(b,false);
    _graph.connect(id1,id2,false);
    if(traverse) update_nets();  // calls traverse_graph()
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

    // Next check for a partially degenerate wire. This is when only one point
    // is on an existing wire, and the two points a,b are collinear with both
    // endpoints of the existing wire.
    // NOTE: If we selected via an endpoint, then select_wire will
    // only return the first wire found. We want to check _all wires_
    // under the point.
    if(w1 != w2)
    {
        // Exactly one wire is not INVALID_WIRE
        Vec<Wire> wires_under;
        bool was_a = false;
        if(w1 != Schematic::INVALID_WIRE)
        {
            // `a` was on wire
            // Get list of all wires under `a`
            wires_under = select_wires(a);
            was_a = true;
        }
        else
        {
            // `b` was on wire
            // Get list of all wires under `b`
            wires_under = select_wires(b);
        }

        for(auto& w0 : wires_under)
        {
            // w0 is the partially overlapped wire, check for collinearity
            Coordinate2 wp1 = _graph.pos(w0.first);
            Coordinate2 wp2 = _graph.pos(w0.second);
            if(collinear(a,b,wp1) && collinear(a,b,wp2))
            {
                // Partially degenerate wire detected
                // Replace point not on wire with closest endpoint of wire
                if(was_a)
                {
                    // `a` was on the wire
                    deg = w0;
                    return WireType::WIRE_PARTIAL_DEGEN_A;
                }
                else
                {
                    // `b` was on the wire
                    deg = w0;
                    return WireType::WIRE_PARTIAL_DEGEN_B;
                }
            }
        }
    }
    return WireType::WIRE_NORMAL;
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
 * Reprocess the whole schematic, no heuristics for new net names. Calls _update_trees().
 * Call this when changes to the schematic are made. Methods like add_wire() and
 * remove_wire() will call it automatically if traverse==true.
 */
void Schematic::update_nets()
{
    // Go through current spanning trees, compare with spanning trees in _nets
    // Remove _nets keys that don't correspond to existing spanning trees
    // If any spanning tree is not a value in _nets, give it a new name, checking
    // for ports
    map<string,Vec<Wire>> nets_new;
    std::set<int> ok_trees;
    std::set<string> ok_nets;
    _update_trees();
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
 * This method clears _vtrees and repopulates it. The _nets data structure
 * is NOT UPDATED by this method. It is called by update_nets().
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


