#include "schematic.h"

int Schematic::add_wire(Coordinate2 a, Coordinate2 b)
{
    return 1;
}

string Schematic::get_netname(int wid)
{
    return "1";
}

vector<int> Schematic::select_net(string netname)
{
    vector<int> wires;
    wires.push_back(1);
    return wires;
}
