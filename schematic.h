#ifndef SCHEMATIC_H
#define SCHEMATIC_H

#include <string>
#include "coordinate2.h"

using std::string;
using std::vector;

class Schematic
{
public:
    string name;
    Schematic() : name{"default"} {}
    Schematic(string name) : name{name} {}

    int add_wire(Coordinate2 a, Coordinate2 b);
    string get_netname(int wid);
    vector<int> select_net(string netname);

private:
    vector<int> wires;

};

#endif // SCHEMATIC_H
