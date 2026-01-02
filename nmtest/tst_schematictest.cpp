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

class SchematicTestFixtureWithWires : public Test
{
protected:
    Schematic sch;
    SchematicTestFixtureWithWires() : sch{"My Schematic"}
    {
        // net 1
        sch.add_wire({16,  8}, {16, 13}, false);
        sch.add_wire({16,  8}, {29,  8}, false);

        // net 2
        sch.add_wire({35,  8}, {45,  8}, false);
        sch.add_wire({45,  8}, {45, 12}, false);

        // net 3
        sch.add_wire({45, 18}, {45, 26}, false);
        sch.add_wire({34, 26}, {45, 26}, false);

        // net 4
        sch.add_wire({16, 26}, {19, 26}, false);
        sch.add_wire({16, 19}, {16, 26}, false);
        sch.add_wire({19, 26}, {25, 26}, false);
        sch.add_wire({19, 26}, {19, 33}, false);
        sch.add_wire({25, 26}, {28, 26}, false);
        sch.add_wire({25, 26}, {25, 29}, false);
        sch.add_wire({25, 29}, {39, 29}, false);
        sch.add_wire({48, 29}, {48, 33}, false);
        sch.add_wire({48, 39}, {48, 45}, false);
        sch.add_wire({35, 45}, {39, 45}, false);
        sch.add_wire({39, 29}, {48, 29}, false);
        sch.add_wire({39, 45}, {48, 45}, false);
        sch.add_wire({39, 29}, {39, 45}, false);

        // net 5
        sch.add_wire({19, 45}, {22, 45}, false);
        sch.add_wire({19, 39}, {19, 42}, false);
        sch.add_wire({19, 42}, {19, 45}, false);
        sch.add_wire({12, 42}, {19, 42}, false);
        sch.add_wire({12, 42}, {12, 51}, false);
        sch.add_wire({12, 51}, {22, 51}, false);
        sch.add_wire({22, 45}, {29, 45}, false);
        sch.add_wire({22, 45}, {22, 51}, false);

        // net 6
        sch.add_wire({60, 26}, {60, 35}, false);
        sch.add_wire({60, 35}, {63, 35}, false);
        sch.add_wire({63, 30}, {63, 35}, false);
        sch.add_wire({63, 30}, {77, 30}, false);
        sch.add_wire({77, 26}, {77, 30}, false);

        // net 7
        sch.add_wire({77, 14}, {77, 20}, false);
        sch.add_wire({60, 14}, {77, 14}, false);
        sch.add_wire({60, 14}, {60, 20}, true);
    }
};


TEST_F(SchematicTestFixture, SchematicNameDefaultAndSetInitialization)
{
    EXPECT_EQ("default",sch_default.name);
    EXPECT_EQ("My Schematic",sch.name);
}

TEST_F(SchematicTestFixture, SchematicAddWireReturnsValidWireId)
{
    sch.add_wire(Coordinate2(0,0),Coordinate2(0,5));
    // string netname = sch.get_netname(wid);
    // EXPECT_EQ("1",netname); // Default net name is "1"
    // vector<int> wids = sch.select_net(netname);
    // EXPECT_THAT(wids,ElementsAre(1));
}

TEST_F(SchematicTestFixtureWithWires, SchematicAddWirePrintsCorrectTrees)
{
    sch.print();
    sch.remove_wire({10,12});
    sch.print();
}

/*
int wid = sch.add_wire(Coordinate2 a,Coordinate2 b);    // Add a wire by start,end
bool status = sch.remove_wire(int wid);                 // Remove wire by id, false if fail
int wid = sch.select_wire(Coordinate2 p);               // Return wire id or -1
vector<int> wids = sch.select_net(string net_name);     // Return list of all wires
                                                        //sharing the net name
vector<int> wids = sch.select_net(Coordinate2 p, true); // Find wire under point, get
                                                        //all wires on its net,
                                                        //true=traverse port nodes
vector<int> wids = sch.select_net(Coordinate2 p, false);// Find wire under point, get
                                                        //all wires on its net,
                                                        //false=don't traverse port
                                                        //nodes
vector<string> netnames = sch.get_netnames();           // Get all net names in sch
int pid = sch.add_port_node(Coordinate2 p);             // Add a port node at p
int pid = sch.select_port_node(Coordinate2 p);          // Find a port node around p
sch.remove_port_node(int pid);                          // Remove port node with id `pid`
sch.print();                                            // Print a netlist

sch.is_connected(int wid1, int wid2);                   // true if wid2 is reachable
                                                        //from wid1
sch.count_leaves(string netname);                       // return number of leaf
                                                        //nodes in tree
*/


