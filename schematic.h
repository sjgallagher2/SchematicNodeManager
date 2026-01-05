#ifndef SCHEMATIC_H
#define SCHEMATIC_H

#include <string>
#include <map>
#include "coordinate2.h"
#include "simplegraph.h"
#include "utils.h"


enum class WireType
{
    WIRE_NORMAL,
    WIRE_DEGENERATE,
    WIRE_PARTIAL_DEGEN_A,  // Wire is partially degenerate because A is on the wire
    WIRE_PARTIAL_DEGEN_B   // Wire is partially degenerate because B is on the wire
};


/* Schematic class for managing wires and ports on a schematic.
 *
 * Usage: A Schematic has a name, a collection of Wire objects, and a collection of
 * Port objects. Add wires with `add_wire()`. If traverse==true, this call automatically
 * runs `update_nets()`. Wires are identified by the internal vertex id's of their
 * endpoints. Because adding or removing a wire can change the graph interconnections,
 * a Wire should be considered _invalid_ after any changes to the schematic.
 *
 * Ports are used to override the netname of a net. They do not interact with wires
 * directly, but they have positions and will rename the net names for any wire they
 * overlap. This is handled in `update_nets()`. Ports must have non-integer names.
 *
 */
class Schematic
{
public:
    using Wire = std::pair<int,int>;  // id1,id2
    using Port = std::pair<Coordinate2, std::string>;  // position, name
    static const Wire INVALID_WIRE;
    std::string name;
    Schematic() : name{"default"} {}
    Schematic(std::string name) : name{name} {}

    // wire and net methods
    Estd::Vec<Wire> get_all_wires() { return _graph.get_all_edges(); }
    Estd::Vec<std::string> get_all_netnames();
    Wire add_wire(Coordinate2 a, Coordinate2 b, bool traverse=true);
    std::string get_netname(Wire w);
    Estd::Vec<Wire> select_net(std::string netname);
    Estd::Vec<Wire> select_net(Coordinate2 p);
    Wire select_wire(Coordinate2 p);
    Estd::Vec<Wire> select_wires(Coordinate2 p);
    bool remove_wire(Wire w, bool traverse=true);
    void update_nets();

    // port methods
    int add_port_node(Port port, bool traverse=true);
    int select_port_node(Coordinate2 p) const;
    Estd::Vec<int> select_port_nodes(std::string port_name) const;
    void remove_port_node(int pid, bool traverse=true);
    void remove_port_nodes(std::string port_name, bool traverse=true);

    void print();

private:
    VertexGraph _graph;
    std::multimap<std::string,Estd::Vec<Wire>> _nets;  // map of netname -> wires
    Estd::Vec<Estd::Vec<int>> _vtrees;      // spanning trees of vertices
    Estd::Vec<Estd::Vec<Wire>> _etrees;
    Estd::Vec<Port> _ports;                 // ports (name and position)
    void _update_trees();                   // reprocess spanning trees
    WireType _degenerate(Coordinate2 a,Coordinate2 b,Wire& deg);
    void _remove_degenerate_wires();

    IdPool _idpool;
};


#endif // SCHEMATIC_H
