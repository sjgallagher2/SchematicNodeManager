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

void Schematic::add_wire(Coordinate2 a, Coordinate2 b, bool traverse)
{
    int id1 = _graph.add(a,false);
    int id2 = _graph.add(b,false);
    _graph.connect(id1,id2,traverse);
}

string Schematic::get_netname(int wid)
{
    return "1";
}

Vec<int> Schematic::select_net(string netname)
{
    Vec<int> wires;
    wires.push_back(1);
    return wires;
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

void Schematic::print()
{
    // for now, print spanning trees
    _update_trees();
    for(auto& tr : _trees)
    {
        print_Vec("Tree: ",tr);
    }
}

/* Update net names.
 * If vid1 (only) or both vid1 and vid2 are supplied, only update nets
 * that involved those vertices. This is much more efficient.
 * If no hints are provided, reprocess the whole schematic, no heuristics
 * for new net names.
 */
void Schematic::update_nets(int vid1, int vid2)
{
    if(vid1 == -1 && vid2 == -1)
    {
        // Go through current spanning trees, compare with spanning trees in _nets
        // Remove _nets keys that don't correspond to existing spanning trees
        // If any spanning tree is not a value in _nets, give it a new name, checking
        // for ports
    }
}


/*
 * Update the spanning trees.
 * This method clears _trees and repopulates it. The _nets data structure
 * is NOT UPDATED by this method.
 */
void Schematic::_update_trees()
{
    _graph.traverse_graph();  // Ensure graph is up to date
    _trees.clear();           // clear existing trees

    // Make a set of vertex ids
    Vec<int> vtxs = _graph.get_all_ids();
    set<int> vtx_set(vtxs.begin(),vtxs.end());
    set<int> vtx_visited;  // vertices already included in a spanning tree

    auto current = vtx_set.begin();
    int timeout = vtx_set.size() + 10;  // never more than this
    while((current != vtx_set.end())&&(timeout >= 0))
    {
        Vec<int> curr_tree = _graph.get_reachable(*current);
        // add this spanning tree
        _trees.push_back(curr_tree);
        // mark vertices as visited
        vtx_visited.insert(*current);
        for(auto& tid : curr_tree) vtx_visited.insert(tid);
        // update current to the next node that hasn't been visited
        auto next = current;
        for(; next != vtx_set.end(); ++next)
        {
            if(vtx_visited.find(*next) == vtx_visited.end())
            {
                current = next;
                break;
            }
        }
        if(current != next)
        {
            // didn't find any unvisited vertices
            current = vtx_set.end();  // could use `next` but this is more explicit
        }
        timeout--;
    }
    if(timeout < 0) throw std::logic_error("Timeout during traversal! Something has gone wrong!");
}


