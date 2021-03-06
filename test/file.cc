#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tglobal.hh"
#include "anc/mpi.hh"
#include "data/datampiio.hh"
#include "object/objsegy.hh"
#define private public
#define protected public
#include "cppfileapi.hh"
#include "file/file.hh"
#undef private
#undef protected

using namespace testing;
using namespace PIOL;

class FileTest : public Test
{
    protected :
    std::shared_ptr<ExSeisPIOL> piol;
    Comm::MPI::Opt opt;

    FileTest()
    {
        opt.initMPI = false;
        piol = std::make_shared<ExSeisPIOL>(opt);
    }
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
//FakeFile to test the constructor of the abstract Interface class
struct FakeReadFile : public File::ReadInterface
{
    struct Opt
    {
        typedef FakeReadFile Type;
    };
    FakeReadFile(std::shared_ptr<ExSeisPIOL> piol_, const std::string name_, std::shared_ptr<Obj::Interface> obj_) : File::ReadInterface(piol_, name_, obj_)
    {
        inc = geom_t(10);
        text = "test";
        nt = 1101U;
        ns = 1010U;
    }

    size_t readNt(void) { return nt; }
    void readTrace(csize_t offset, csize_t sz, trace_t * trace, File::Param * prm, size_t skip) const {}
    void readParam(csize_t offset, csize_t sz, File::Param * prm, size_t skip) const {}

    void readTrace(csize_t sz, csize_t * offset, trace_t * trace, File::Param * prm, size_t skip) const {}
    void readTraceNonMono(csize_t sz, csize_t * offset, trace_t * trace, File::Param * prm, size_t skip) const {}
    void readParam(csize_t sz, csize_t * offset, File::Param * prm, size_t skip) const {}
};

struct FakeWriteFile : public File::WriteInterface
{
    struct Opt
    {
        typedef FakeWriteFile Type;
    };
    FakeWriteFile(std::shared_ptr<ExSeisPIOL> piol_, const std::string name_, std::shared_ptr<Obj::Interface> obj_) : File::WriteInterface(piol_, name_, obj_)
    {
        inc = geom_t(10);
        text = "test";
        nt = 1101U;
        ns = 1010U;
    }

    void writeText(const std::string text_) {}
    void writeNs(csize_t ns_) {}
    void writeNt(csize_t nt_) {}
    void writeInc(const geom_t inc_) {}
    void writeTrace(csize_t offset, csize_t sz, trace_t * trace, const File::Param * prm, size_t skip) {}
    void writeParam(csize_t offset, csize_t sz, const File::Param * prm, size_t skip) {}

    void writeTrace(csize_t sz, csize_t * offset, trace_t * trace, const File::Param * prm, size_t skip) {}
    void writeParam(csize_t sz, csize_t * offset, const File::Param * prm, size_t skip) {}
};

#pragma GCC diagnostic pop
void compareConstructor(ExSeisPIOL * piol, FakeReadFile & fake)
{
    EXPECT_EQ(piol, fake.piol.get());
    EXPECT_EQ(tempFile, fake.name);
    EXPECT_EQ(1010U, fake.readNs());
    EXPECT_EQ(geom_t(10), fake.readInc());
    EXPECT_EQ("test", fake.readText());
}

//We test the constructor
TEST_F(FileTest, Constructor)
{
    std::shared_ptr<Obj::Interface> obj = nullptr;
    FakeReadFile fake(piol, tempFile, obj);
    EXPECT_EQ(nullptr, fake.obj);
    compareConstructor(piol.get(), fake);
}

typedef FileTest FileDeathTest;

//In this test we pass the MPI-IO Data Options class within an invalid name
TEST_F(FileDeathTest, BadNameConstructor)
{
    File::ReadDirect file(piol, notFile);
    EXPECT_EQ(piol, file->piol);
    EXPECT_EQ(notFile, file->name);
    EXPECT_EXIT(piol->isErr(), ExitedWithCode(EXIT_FAILURE), ".*8 3 Fatal Error in PIOL. . Dumping Log 0");
}
