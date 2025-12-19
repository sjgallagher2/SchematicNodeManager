#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include "../coordinate2.h"

using namespace testing;

class Coordinate2TestFixture : public Test
{
protected:
    const double PREC = 1e-5;
    double xd = 1.532;
    double yd = -6.661;
    float xf = -9.112;
    float yf = 4.795;
    Coordinate2 porigin, p1d, p1f, p1;

    Coordinate2TestFixture() : p1d{xd,yd},p1f{xf,yf},p1{xd,yd} {}
};

TEST_F(Coordinate2TestFixture, Coordinate2FloatDoubleInitWorks)
{
    EXPECT_NEAR(xd,p1d.x,PREC);  // double
    EXPECT_NEAR(yd,p1d.y,PREC);
    EXPECT_NEAR(xf,p1f.x,PREC);  // float
    EXPECT_NEAR(yf,p1f.y,PREC);
}
TEST_F(Coordinate2TestFixture, Coordinate2DefaultInitIsOrigin)
{
    EXPECT_NEAR(0,porigin.x,PREC);  // default (0,0)
    EXPECT_NEAR(0,porigin.y,PREC);
}
TEST_F(Coordinate2TestFixture, Coordinate2PrecThrowsExceptionOnLEZero)
{
    ASSERT_THROW(p1.prec(-1.0),std::invalid_argument);
    ASSERT_THROW(p1.prec(0.0),std::invalid_argument);
}
TEST_F(Coordinate2TestFixture, Coordinate2CopyConstructorWorks)
{
    Coordinate2 p2(p1);     // Copy constructor
    EXPECT_NEAR(p1.x,p2.x,PREC);
    EXPECT_NEAR(p1.y,p2.y,PREC);
}
TEST_F(Coordinate2TestFixture, Coordinate2AssignCopyWorks)
{
    Coordinate2 p2 = p1;  // Copy assignment
    EXPECT_NEAR(p1.x,p2.x,PREC);
    EXPECT_NEAR(p1.y,p2.y,PREC);
}
TEST_F(Coordinate2TestFixture, Coordinate2ComparisonWithDefaultPrec)
{
    Coordinate2 p2 = p1;
    ASSERT_TRUE(p1 == p2);
}
TEST_F(Coordinate2TestFixture, Coordinate2ComparisonWithOtherPrec)
{
    Coordinate2 p2 = p1;
    p2.x += 2e-5;
    p1.prec(1e-5);
    ASSERT_FALSE(p1 == p2);
    p1.prec(3e-5);
    ASSERT_TRUE(p1 == p2);
}
TEST_F(Coordinate2TestFixture, Coordinate2DistanceToSamePointIsZero)
{
    Coordinate2 p2 = p1;
    ASSERT_NEAR(0.0,p1.distance(p2),PREC);
}
TEST_F(Coordinate2TestFixture, Coordinate2DistanceToOriginIsCorrect)
{
    ASSERT_NEAR(6.83490636366,p1.distance(porigin),PREC);
}
TEST_F(Coordinate2TestFixture, Coordinate2MagnitudeIsDistanceToOrigin)
{
    double p1mag = p1.magnitude();
    double distorigin = p1.distance(porigin);
    ASSERT_NEAR(p1mag,distorigin,PREC);
}
TEST_F(Coordinate2TestFixture, Coordinate2VectorAngleIsCorrect)
{
    EXPECT_NEAR(- 1.344732216,p1.angle(),1e-8);
    EXPECT_NEAR(-77.047480572,p1.angle(true),1e-8);
}
TEST_F(Coordinate2TestFixture, Coordinate2VectorAngleFailsOnOrigin)
{
    ASSERT_THROW(porigin.angle(),std::logic_error);
}
TEST_F(Coordinate2TestFixture, Coordinate2RelativeAngleIsCorrect)
{
    Coordinate2 p2(0.5,9.1);
    EXPECT_NEAR(- 1.505411584,p1.angle(p2),1e-8);
    EXPECT_NEAR(-86.253730219,p1.angle(p2,true),1e-8);
}
TEST_F(Coordinate2TestFixture, Coordinate2RelativeAngleFailsOnOrigin)
{
    ASSERT_THROW(p1.angle(porigin),std::logic_error);
}
TEST_F(Coordinate2TestFixture, Coordinate2VectorNegativeIsCorrect)
{
    Coordinate2 negp1 = p1;
    negp1.x *= -1;
    negp1.y *= -1;
    EXPECT_TRUE(-p1 == negp1);
}
// TEST_F(Coordinate2TestFixture, Coordinate2)
// {

// }

/*
SCALAR ADDITION

SCALAR SUBTRACTION

SCALAR MULTIPLICATION

SCALAR DIVISION

VECTOR ADDITION

VECTOR SUBTRACTION

SCALAR ADDITION WITH ASSIGN

SCALAR SUBTRACTION WITH ASSIGN

SCALAR MULTPLICATION WITH ASSIGN

SCALAR DIVISION WITH ASSIGN

VECTOR ADDITION WITH ASSIGN

VECTOR SUBTRACTION WITH ASSIGN

 */

