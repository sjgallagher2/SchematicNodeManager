#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <utility>
#include "../coordinate2.h"
#include "../schematic.h"

using namespace testing;
using std::string;
using std::vector;
using std::pair;
using Wire = std::pair<int,int>;

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

TEST_F(SchematicTestFixture, SchematicAddWireReturnsValidWire)
{
    // For any isolated wire, the wire is guaranteed to stay the same after adding
    Wire w1 = sch.add_wire(Coordinate2(0,0),Coordinate2(0,5));
    ASSERT_NE(w1,Schematic::INVALID_WIRE);
    ASSERT_NO_THROW(sch.get_netname(w1));  // if w1 does not exist, this will throw
    string net1 = sch.get_netname(w1);
    EXPECT_THAT(sch.get_all_netnames(),Contains(net1));

    // Add second wire on endpoint, should have same net
    Wire w2 = sch.add_wire(Coordinate2(0,0),Coordinate2(5,0));
    string net2 = sch.get_netname(w2);
    EXPECT_EQ(net1,net2);

    // Add third wire, isolated, should be a new net
    Wire w3 = sch.add_wire(Coordinate2(1,1),Coordinate2(1,4));
    string net3 = sch.get_netname(w3);
    EXPECT_NE(net1,net3);

    // Add fourth wire, isolated, do not traverse after adding
    Wire w4 = sch.add_wire(Coordinate2(16,19),Coordinate2(17,20),false);
    EXPECT_THROW(sch.get_netname(w4),std::invalid_argument);
    sch.update_nets();
    EXPECT_NO_THROW(sch.get_netname(w4));

    // Add fifth wire, along length of w3, should split w3 and have same net
    // So w3 should no longer be found
    Wire w5 = sch.add_wire(Coordinate2(1,2.5),Coordinate2(3,2.5));
    // w5 is still in tact
    string net5 = sch.get_netname(w5);
    EXPECT_EQ(net5,net3);
    // w3 is split
    EXPECT_THROW(sch.get_netname(w3),std::invalid_argument);
    // New wire is w3.first and w5.first, hack it together
    Wire w3_1(w3.first,w5.first);
    Wire w3_2(w5.first,w3.second);
    // These should be on the same net
    EXPECT_NO_THROW(sch.get_netname(w3_1));
    EXPECT_NO_THROW(sch.get_netname(w3_2));
    string net3_1 = sch.get_netname(w3_1);
    string net3_2 = sch.get_netname(w3_2);
    EXPECT_EQ(net3_1,net3_2);
    EXPECT_EQ(net3_1,net5);

    // Adding degenerate wires
    // Wire overlapping another (w5)
    Wire w6 = sch.add_wire(Coordinate2(1,2.5),Coordinate2(3,2.5));
    EXPECT_EQ(w5,w6);
    // Wire degenerate on top of another (w2)
    Wire w7 = sch.add_wire(Coordinate2(1,0),Coordinate2(4,0));
    // A degenerate wire should not be added and should not change the
    // schematic.
    EXPECT_EQ(w7,Schematic::INVALID_WIRE);
    EXPECT_THROW(sch.get_netname(w7),std::invalid_argument);
    EXPECT_NO_THROW(sch.get_netname(w2));
    // Wire of zero length
    Wire w8 = sch.add_wire(Coordinate2(0,0),Coordinate2(0,0));
    EXPECT_EQ(w8,Schematic::INVALID_WIRE);
    EXPECT_NO_THROW(sch.get_netname(w1));  // make sure w1 (which shares 0,0) is unchanged

    // Partially degenerate wire, one wire extends another
    // w9_1 should not be invalid, but it is no longer a Wire
    // Instead, the wire should be w1.first,w9_1.second
    Wire w9_1 = sch.add_wire(Coordinate2(0,5),Coordinate2(0,6));
    EXPECT_NE(w9_1,Schematic::INVALID_WIRE);
    EXPECT_THROW(sch.get_netname(w9_1),std::invalid_argument);
    EXPECT_NO_THROW(sch.get_netname(Wire{w1.first,w9_1.second}));

    // Partially degenerate wire, one wire completely subsumes another
    Wire w9_2 = sch.add_wire(Coordinate2(0,0),Coordinate2(0,7));
    EXPECT_NO_THROW(sch.get_netname(w9_2));
    EXPECT_NE(w9_2,Schematic::INVALID_WIRE);

    // Partially degenerate wire, one wire partially overlaps another
    Wire w9_3 = sch.add_wire(Coordinate2(0,3),Coordinate2(0,8));
    EXPECT_THROW(sch.get_netname(w9_3),std::invalid_argument);
    EXPECT_NO_THROW(sch.get_netname(Wire{w1.first,w9_3.second}));
    EXPECT_NE(w9_3,Schematic::INVALID_WIRE);

    //  Partially degenerate wire, the new wire subsumes multiple other wires
    Wire w9_4 = sch.add_wire({-5,-10},{-3,-10},false);
    Wire w9_5 = sch.add_wire({-2,-10},{-1,-10},false);
    Wire w9_6 = sch.add_wire({0,-10},{2,-10},false);
    Wire w9_7 = sch.add_wire({3,-10},{5,-10},false);
    Wire w9_8 = sch.add_wire({-8,-10},{10,-10},true);
    // w9_8 subsumes all the others
    EXPECT_NO_THROW(sch.get_netname(w9_8));
    EXPECT_THROW(sch.get_netname(w9_4),std::invalid_argument);
    EXPECT_THROW(sch.get_netname(w9_5),std::invalid_argument);
    EXPECT_THROW(sch.get_netname(w9_6),std::invalid_argument);
    EXPECT_THROW(sch.get_netname(w9_7),std::invalid_argument);

    // Adding wire that gets split in its middle
    // [split by (0,0)]
    Wire w10 = sch.add_wire(Coordinate2(-1,-1),Coordinate2(0.5,0.5));
    EXPECT_THROW(sch.get_netname(w10),std::invalid_argument); // This should have been split
}

TEST_F(SchematicTestFixtureWithWires, SchematicTestRemoveWireWorks)
{
    using std::cout;
    using std::endl;
    using Estd::Vec;
    using std::string;
    // Verify default nets, number of wires
    EXPECT_THAT(sch.get_all_netnames(),ElementsAre("0","1","2","3","4","5","6"));
    EXPECT_EQ(sch.get_all_wires().size(),35);

    // Remove w0, net0 still has w1 in it
    Wire w = sch.select_wire({16,9}); // select w0
    EXPECT_NE(w,Schematic::INVALID_WIRE);  // properly selected
    sch.remove_wire(w);
    EXPECT_THAT(sch.get_all_netnames(),ElementsAre("0","1","2","3","4","5","6"));
    EXPECT_EQ(sch.get_all_wires().size(),34);

    // Remove w1, net goes away
    w = sch.select_wire({20,8}); // select w1
    string nn1 = sch.get_netname(w);  // netname to be removed
    EXPECT_NE(w,Schematic::INVALID_WIRE);  // properly selected
    sch.remove_wire(w);
    Vec<string> expec = {"0","1","2","3","4","5","6"};
    expec.erase(Estd::find(expec,nn1));
    EXPECT_EQ(sch.get_all_netnames(),expec);
    EXPECT_EQ(sch.get_all_wires().size(),33);

    // Add a wire to other net, shouldn't change anything
    sch.add_wire({77,14},{80,14});
    EXPECT_EQ(sch.get_all_netnames(),expec);
    EXPECT_EQ(sch.get_all_wires().size(),34);

    // Add a wire in isolation, should get prev net back, back to start
    sch.add_wire({0,0},{10,0});
    EXPECT_THAT(sch.get_all_netnames(),ElementsAre("0","1","2","3","4","5","6"));
    EXPECT_EQ(sch.get_all_wires().size(),35);
}

TEST_F(SchematicTestFixtureWithWires, SchematicTestAddPortRenamesNets)
{
    //
}

TEST_F(SchematicTestFixtureWithWires, SchematicTestAddingAndRemovingPorts)
{
    sch.print();
    std::cout << "******************\n";
    int port1 = sch.add_port_node({{16,8},"Gnd"});
    sch.print();
    std::cout << "******************\n";
    int port2 = sch.add_port_node({{60,30},"Gnd"});
    sch.print();
    std::cout << "******************\n";
    sch.remove_port_node(port1);
    sch.print();
    std::cout << "******************\n";
}



