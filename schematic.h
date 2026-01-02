#ifndef SCHEMATIC_H
#define SCHEMATIC_H

#include <string>
#include <map>
#include "coordinate2.h"
#include "simplegraph.h"
#include "utils.h"


/* Schematic class for managing wires and ports on a schematic.
 *
 * NOTE: Ports CANNOT have integer numbers for names. Besides, why would you do that?
 *
 */

class Schematic
{
public:
    using Wire = std::pair<int,int>;  // id1,id2
    std::string name;
    Schematic() : name{"default"} {}
    Schematic(std::string name) : name{name} {}

    Estd::Vec<Wire> get_all_wires() { return _graph.get_all_edges(); }
    Wire add_wire(Coordinate2 a, Coordinate2 b, bool traverse=true);
    Estd::Vec<std::string> get_all_netnames();
    std::string get_netname(Wire w);
    Estd::Vec<Wire> select_net(std::string netname);
    Estd::Vec<Wire> select_net(Coordinate2 p);
    Wire select_wire(Coordinate2 p);
    bool remove_wire(Wire w, bool traverse=true);
    int add_port_node(Coordinate2 p);
    int select_port_node(Coordinate2 p);
    bool remove_port_node(int pid);
    void print();

    void update_nets(int vhint1=-1,int vhint2=-1);

private:
    Estd::Vec<Wire> _wires;  // edges in current schematic
    VertexGraph _graph;
    std::map<std::string,Estd::Vec<Wire>> _nets;  // map of netname -> wires
    Estd::Vec<Estd::Vec<int>> _vtrees;      // spanning trees of vertices
    Estd::Vec<Estd::Vec<Wire>> _etrees;
    void _update_trees();                  // reprocess spanning trees
    IdPool _idpool;
};


#endif // SCHEMATIC_H
