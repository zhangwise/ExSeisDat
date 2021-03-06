#include <memory>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tglobal.hh"
#include "anc/mpi.hh"
#define private public
#define protected public
#include "data/data.hh"
#undef private
#undef protected

using namespace testing;
using namespace PIOL;
class DataTest : public Test
{
    protected :
    std::shared_ptr<ExSeisPIOL> piol;
    Comm::MPI::Opt opt;
    DataTest()
    {
        opt.initMPI = false;
        piol = std::make_shared<ExSeisPIOL>(opt);
    }
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
struct FakeData : public Data::Interface
{
    FakeData(const std::shared_ptr<ExSeisPIOL> piol_, const std::string name_) : Data::Interface(piol_, name_)
    {

    }

    size_t getFileSz() const
    {
        return 0U;
    }

    void read(csize_t offset, csize_t sz, uchar * d) const {}
    void read(csize_t offset, csize_t bsz, csize_t osz, csize_t sz, uchar * d) const {}
    void read(csize_t bsz, csize_t sz, csize_t * offset, uchar * d) const {}
    void setFileSz(csize_t sz) const {}
    void write(csize_t offset, csize_t sz, const uchar * d) const {}
    void write(csize_t offset, csize_t bsz, csize_t osz, csize_t sz, const uchar * d) const {}
    void write(csize_t bsz, csize_t sz, csize_t * offset, const uchar * d) const {}
};
#pragma GCC diagnostic pop

TEST_F(DataTest, Constructor)
{
    FakeData fake(piol, notFile);
    EXPECT_EQ(piol, fake.piol);
    EXPECT_EQ(notFile, fake.name);
}
