#ifndef SCHEMATIC_H
#define SCHEMATIC_H

#include <string>
#include "coordinate2.h"
#include "simplegraph.h"

using std::string;
using std::vector;

/*
 *
 */

class Schematic
{
public:
    string name;
    Schematic() : name{"default"} {}
    Schematic(string name) : name{name} {}

    int add_wire(Coordinate2 a, Coordinate2 b);
    string get_netname(int wid);
    vector<int> select_net(string netname);
    bool remove_wire(int wid);
    vector<int> select_net(Coordinate2 p, bool traverse=true);
    vector<string> get_netnames();
    int add_port_node(Coordinate2 p);
    int select_port_node(Coordinate2 p);
    bool remove_port_node(int pid);
    void print();
    bool is_connected(int wid1, int wid2);


private:
    vector<int> _wires;
    VertexGraph _graph;

};

#endif // SCHEMATIC_H
