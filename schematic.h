#ifndef SCHEMATIC_H
#define SCHEMATIC_H

#include <string>
#include <map>
#include "coordinate2.h"
#include "simplegraph.h"
#include "utils.h"


/*
 *
 */

class Schematic
{
public:
    std::string name;
    Schematic() : name{"default"} {}
    Schematic(std::string name) : name{name} {}

    void add_wire(Coordinate2 a, Coordinate2 b, bool traverse=true);
    std::string get_netname(int wid);
    Estd::Vec<int> select_net(std::string netname);
    bool remove_wire(int wid);
    Estd::Vec<int> select_net(Coordinate2 p, bool traverse=true);
    Estd::Vec<std::string> get_netnames();
    int add_port_node(Coordinate2 p);
    int select_port_node(Coordinate2 p);
    bool remove_port_node(int pid);
    void print();

    void update_nets(int vid1=-1,int vid2=-1);

private:
    Estd::Vec<std::pair<int,int>> _wires;  // edges in current schematic
    VertexGraph _graph;
    std::map<std::string,Estd::Vec<int>> _nets;  // map of netname -> graphnodes
    Estd::Vec<Estd::Vec<int>> _trees;      // spanning trees
    void _update_trees();                  // reprocess spanning trees
};

#endif // SCHEMATIC_H
