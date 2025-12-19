#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include "../coordinate2.h"
#include "../schematic.h"

using namespace testing;
using std::string;
using std::vector;

class SchematicTestFixture : public Test
{
protected:
    Schematic sch, sch_default;

    SchematicTestFixture() : sch{"My Schematic"}, sch_default{} {}
};

TEST_F(SchematicTestFixture, SchematicNameDefaultAndSetInitialization)
{
    EXPECT_EQ("default",sch_default.name);
    EXPECT_EQ("My Schematic",sch.name);
}

TEST_F(SchematicTestFixture, SchematicAddWireReturnsValidWireId)
{
    int wid = sch.add_wire(Coordinate2(0,0),Coordinate2(0,5));
    EXPECT_EQ(1, wid);  // Default wire id is 1
    string netname = sch.get_netname(wid);
    EXPECT_EQ("1",netname); // Default net name is "1"
    vector<int> wids = sch.select_net(netname);
    EXPECT_THAT(wids,ElementsAre(1));
}

/*
int wid = sch.add_wire(Coordinate2 a,Coordinate2 b);  // Add a wire by start,end
bool status = sch.remove_wire(int wid);  // remove wire by id, false if fail
int wid = sch.select_wire(Coordinate2 p);   // Return wire id or -1
vector<int> wids = sch.select_net(string net_name);  // Return list of all
// wires sharing the net name
vector<int> wids = sch.select_net(Coordinate2 p, true);  // Find wire under
// point, get all wires on its net, true=traverse port nodes
vector<int> wids = sch.select_net(Coordinate2 p, false);  // Find wire under
// point, get all wires on its net, false=don't traverse port nodes
vector<string> netnames = sch.get_netnames();  // Get all net names in sch
int pid = sch.add_port_node(Coordinate2 p);    // Add a port node at p
int pid = sch.select_port_node(Coordinate2 p); // Find a port node around p
sch.remove_port_node(int pid);  // Remove port node with id `pid`
sch.print();  // Print a netlist

sch.is_connected(int wid1, int wid2); // true if wid2 is reachable from wid1
sch.count_leaves(string netname);     // return number of leaf nodes in tree
*/
