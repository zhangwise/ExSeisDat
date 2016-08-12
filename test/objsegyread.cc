#include "objsegytest.hh"
TEST_F(ObjSpecTest, SEGYHORead)
{
    makeSEGY();
    readHOPatternTest(7, 107);
    readHOPatternTest(10, 46);
    readHOPatternTest(0, 0);
}

TEST_F(ObjSpecTest, SEGYDOMDReadSingle)
{
    makeSEGY();
    readTest<true>(10U, 1U, 200, 13, 117);
    readTest<true>(10U, 1U, 200, 13, 13);
}

TEST_F(ObjSpecTest, SEGYDOMDReadZeroNt)
{
    makeSEGY();
    readTest<true>(10U, 0U, 2000);
}

TEST_F(ObjSpecTest, SEGYDOMDReadZeroNs)
{
    makeSEGY();
    readTest<true>(10U, 100U, 0U);
}

TEST_F(ObjSpecTest, SEGYDOMDRead)
{
    makeSEGY();
    readTest<true>(10U, 100U, 2000);
}

TEST_F(ObjSpecTest, FarmSEGYDOMDBigRead)
{
    makeSEGY();
    readTest<true>(10U, 300000, 5000);
}

TEST_F(ObjSpecTest, SEGYDODFReadSingle)
{
    makeSEGY();
    readTest<false>(10U, 1U, 200, 13, 117);
    readTest<false>(10U, 1U, 200, 13, 13);
}

TEST_F(ObjSpecTest, SEGYDODFReadZeroNt)
{
    makeSEGY();
    readTest<false>(10U, 0U, 2000);
}

TEST_F(ObjSpecTest, SEGYDODFReadZeroNs)
{
    makeSEGY();
    readTest<false>(10U, 100U, 0U);
}

TEST_F(ObjSpecTest, SEGYDODFRead)
{
    makeSEGY();
    readTest<false>(10U, 100U, 2000);
}

TEST_F(ObjSpecTest, FarmSEGYDODFBigRead)
{
    makeSEGY();
    readTest<false>(10U, 300000, 5000);
}
