#include <iconv.h>
#include <string.h>
#include <memory>
#include <typeinfo>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tglobal.hh"
#include "anc/mpi.hh"
#include "data/datampiio.hh"
#include "object/objsegy.hh"
#include "share/units.hh"
#include "share/segy.hh"
#include "share/datatype.hh"

#define private public
#define protected public
#include "cppfileapi.hh"
#include "file/filesegy.hh"
#include "segymdextra.hh"
#undef private
#undef protected

/*namespace PIOL { namespace File {
extern int16_t calcScale(const coord_t coord);
extern int16_t scalComp(int16_t scal1, int16_t scal2);
extern void setCoord(const File::Coord item, const coord_t coord, const int16_t scale, uchar * buf);
extern void setGrid(const File::Grid item, const grid_t grid, uchar * buf);
extern int16_t deScale(const geom_t val);
}}*/

using namespace testing;
using namespace PIOL;
using PIOL::File::deScale;
using File::coord_t;
using File::grid_t;
using File::calcScale;
using File::scalComp;
using File::setCoord;
using File::setGrid;


enum Hdr : size_t
{
    Increment  = 3216U,
    NumSample  = 3220U,
    Type       = 3224U,
    Sort       = 3228U,
    Units      = 3254U,
    SEGYFormat = 3500U,
    FixedTrace = 3502U,
    Extensions = 3504U,
};

enum TrHdr : size_t
{
    SeqNum      = 0U,
    SeqFNum     = 4U,
    ORF         = 8U,
    TORF        = 12U,
    RcvElv      = 40U,
    SurfElvSrc  = 44U,
    SrcDpthSurf = 48U,
    DtmElvRcv   = 52U,
    DtmElvSrc   = 56U,
    WtrDepSrc   = 60U,
    WtrDepRcv   = 64U,
    ScaleElev   = 68U,
    ScaleCoord  = 70U,
    xSrc        = 72U,
    ySrc        = 76U,
    xRcv        = 80U,
    yRcv        = 84U,
    xCMP        = 180U,
    yCMP        = 184U,
    il          = 188U,
    xl          = 192U
};

class MockObj : public Obj::Interface
{
    public :
    MockObj(std::shared_ptr<ExSeisPIOL> piol_, const std::string name_, std::shared_ptr<Data::Interface> data_)
               : Obj::Interface(piol_, name_, data_) {}
    MOCK_CONST_METHOD0(getFileSz, size_t(void));
    MOCK_CONST_METHOD1(readHO, void(uchar *));
    MOCK_CONST_METHOD1(setFileSz, void(csize_t));
    MOCK_CONST_METHOD1(writeHO, void(const uchar *));
#warning TODO: Separate out groups of functions to separate files
#warning Not covered yet beyond sz=1
    MOCK_CONST_METHOD4(readDOMD, void(csize_t, csize_t, csize_t, uchar *));
    MOCK_CONST_METHOD4(writeDOMD, void(csize_t, csize_t, csize_t, const uchar *));

    MOCK_CONST_METHOD4(readDODF, void(csize_t, csize_t, csize_t, uchar *));
    MOCK_CONST_METHOD4(writeDODF, void(csize_t, csize_t, csize_t, const uchar *));
    MOCK_CONST_METHOD4(readDO, void(csize_t, csize_t, csize_t, uchar *));
    MOCK_CONST_METHOD4(writeDO, void(csize_t, csize_t, csize_t, const uchar *));

    MOCK_CONST_METHOD4(readDO, void(csize_t, csize_t, csize_t *, uchar *));
    MOCK_CONST_METHOD4(writeDO, void(csize_t, csize_t, csize_t *, const uchar *));
    MOCK_CONST_METHOD4(readDODF, void(csize_t, csize_t, csize_t *, uchar *));
    MOCK_CONST_METHOD4(writeDODF, void(csize_t, csize_t, csize_t *, const uchar *));

#warning Not covered yet.
    MOCK_CONST_METHOD4(readDOMD, void(csize_t, csize_t, csize_t *, uchar *));
    MOCK_CONST_METHOD4(writeDOMD, void(csize_t, csize_t, csize_t *, const uchar *));
};

struct FileSEGYTest : public Test
{
    std::shared_ptr<ExSeisPIOL> piol;
    Comm::MPI::Opt opt;
    bool testEBCDIC;
    std::string testString = {"This is a string for testing EBCDIC conversion etc."};
    std::unique_ptr<File::Direct> file;
    std::vector<uchar> tr;
    size_t nt = 40U;
    size_t ns = 200U;
    int inc = 10;
    csize_t format = 5;
    std::vector<uchar> ho;
    std::shared_ptr<MockObj> mock;

    FileSEGYTest()
    {
        testEBCDIC = false;
        file = nullptr;
        opt.initMPI = false;
        piol = std::make_shared<ExSeisPIOL>(opt);
        ho.resize(SEGSz::getHOSz());
    }

    ~FileSEGYTest()
    {
        Mock::VerifyAndClearExpectations(&mock);
    }

    template <bool WRITE = true, bool OPTS = false>
    void makeSEGY(std::string name)
    {
        if (file.get() != nullptr)
            file.reset();
        FileMode mode = (WRITE ? FileMode::Test : FileMode::Read);
        if (OPTS)
        {
            File::SEGY::Opt fopt;
            Obj::SEGY::Opt oopt;
            Data::MPIIO::Opt dopt;
            file = std::make_unique<File::Direct>(piol, name, fopt, oopt, dopt, mode);
        }
        else
            file = std::make_unique<File::Direct>(piol, name, mode);

        piol->isErr();

        if (WRITE)
            writeHO<false>();
    }

    template <bool WRITE = true, bool callHO = true>
    void makeMockSEGY()
    {
        if (file.get() != nullptr)
            file.reset();
        if (mock != nullptr)
            mock.reset();
        mock = std::make_shared<MockObj>(piol, notFile, nullptr);
        piol->isErr();
        Mock::AllowLeak(mock.get());

        ho.resize(SEGSz::getHOSz());

        if (!WRITE)
        {
            EXPECT_CALL(*mock, getFileSz()).Times(Exactly(1)).WillOnce(Return(SEGSz::getHOSz() + nt*SEGSz::getDOSz(ns)));
            initReadHOMock(testEBCDIC);
        }
        else
            EXPECT_CALL(*mock, getFileSz()).Times(Exactly(1)).WillOnce(Return(0U));

        auto sfile = std::make_shared<File::SEGY>(piol, notFile, mock, (WRITE ? FileMode::Test : FileMode::Read));
        file = std::make_unique<File::Direct>();
        file->file = std::move(sfile);
        file->rule = std::make_shared<File::Rule>(true, true);

        if (WRITE)
        {
            if (callHO)
            {
                piol->isErr();
                writeHO<true>();
            }
            else
            {
                file->file->nt = nt;
                file->file->ns = ns;
            }
        }
    }

    void initReadHOMock(bool testEBCDIC)
    {
        if (testEBCDIC)
        {
            // Create an EBCDID string to convert back to ASCII in the test
            size_t tsz = testString.size();
            size_t tsz2 = tsz;
            char * t = &testString[0];
            char * newText = reinterpret_cast<char *>(ho.data());
            iconv_t toAsc = iconv_open("EBCDICUS//", "ASCII//");
            ::iconv(toAsc, &t, &tsz, &newText, &tsz2);
            iconv_close(toAsc);
        }
        else
        {
            for (size_t i = 0; i < testString.size(); i++)
                ho[i] = testString[i];
        }
        for (size_t i = testString.size(); i < SEGSz::getTextSz(); i++)
            ho[i] = ho[i % testString.size()];

        ho[NumSample] = ns >> 8 & 0xFF;
        ho[NumSample+1] = ns & 0xFF;
        ho[Increment] = inc >> 8 & 0xFF;
        ho[Increment+1] = inc & 0xFF;
        ho[Type+1] = format;
        EXPECT_CALL(*mock, readHO(_)).Times(Exactly(1)).WillOnce(SetArrayArgument<0>(ho.begin(), ho.end()));
    }

    void initTrBlock()
    {
        tr.resize(nt * SEGSz::getMDSz());
        for (size_t i = 0; i < nt; i++)
        {
            uchar * md = &tr[i * SEGSz::getMDSz()];
            getBigEndian(ilNum(i), &md[il]);
            getBigEndian(xlNum(i), &md[xl]);

            int16_t scale;
            int16_t scal1 = deScale(xNum(i));
            int16_t scal2 = deScale(yNum(i));

            if (scal1 > 1 || scal2 > 1)
                scale = std::max(scal1, scal2);
            else
                scale = std::min(scal1, scal2);

            getBigEndian(scale, &md[ScaleCoord]);
            getBigEndian(int32_t(std::lround(xNum(i)/scale)), &md[xSrc]);
            getBigEndian(int32_t(std::lround(yNum(i)/scale)), &md[ySrc]);
        }
    }

    void initReadTrMock(size_t ns, size_t offset)
    {
        std::vector<uchar>::iterator iter = tr.begin() + offset*SEGSz::getMDSz();
        EXPECT_CALL(*mock.get(), readDOMD(offset, ns, 1U, _))
                    .Times(Exactly(1))
                    .WillRepeatedly(SetArrayArgument<3>(iter, iter + SEGSz::getMDSz()));

        File::Param prm(1U);
        file->readParam(offset, 1U, &prm);
        ASSERT_EQ(ilNum(offset), File::getPrm<llint>(0U, File::Meta::il, &prm));
        ASSERT_EQ(xlNum(offset), File::getPrm<llint>(0U, File::Meta::xl, &prm));

        ASSERT_DOUBLE_EQ(xNum(offset), File::getPrm<geom_t>(0U, File::Meta::xSrc, &prm));
        ASSERT_DOUBLE_EQ(yNum(offset), File::getPrm<geom_t>(0U, File::Meta::ySrc, &prm));
    }

    void initReadTrHdrsMock(size_t ns, size_t tn)
    {
        EXPECT_CALL(*mock.get(), readDOMD(0, ns, tn, _))
                    .Times(Exactly(1))
                    .WillRepeatedly(SetArrayArgument<3>(tr.begin(), tr.end()));

        File::Param prm(tn);
        file->readParam(0, tn, &prm);

        for (size_t i = 0; i < tn; i++)
        {
            ASSERT_EQ(ilNum(i), File::getPrm<llint>(i, File::Meta::il, &prm));
            ASSERT_EQ(xlNum(i), File::getPrm<llint>(i, File::Meta::xl, &prm));

            ASSERT_DOUBLE_EQ(xNum(i), File::getPrm<geom_t>(i, File::Meta::xSrc, &prm));
            ASSERT_DOUBLE_EQ(yNum(i), File::getPrm<geom_t>(i, File::Meta::ySrc, &prm));
        }
    }

    template <bool MOCK = true>
    void writeHO()
    {
        if (MOCK)
        {
            size_t fsz = SEGSz::getHOSz() + nt*SEGSz::getDOSz(ns);
            EXPECT_CALL(*mock, setFileSz(fsz)).Times(Exactly(1));

            for (size_t i = 0U; i < std::min(testString.size(), SEGSz::getTextSz()); i++)
                ho[i] = testString[i];

            ho[NumSample+1] = ns & 0xFF;
            ho[NumSample] = ns >> 8 & 0xFF;
            ho[Increment+1] = inc & 0xFF;
            ho[Increment] = inc >> 8 & 0xFF;
            ho[Type+1] = format;
            ho[3255U] = 1;
            ho[3500U] = 1;
            ho[3503U] = 1;
            ho[3505U] = 0;

            EXPECT_CALL(*mock, writeHO(_)).Times(Exactly(1)).WillOnce(check0(ho.data(), SEGSz::getHOSz()));
        }

        file->writeNt(nt);
        piol->isErr();

        file->writeNs(ns);
        piol->isErr();

        file->writeInc(geom_t(inc*SI::Micro));
        piol->isErr();

        file->writeText(testString);
        piol->isErr();
    }

    void writeTrHdrGridTest(size_t offset)
    {
        std::vector<uchar> tr(SEGSz::getMDSz());
        getBigEndian(ilNum(offset), tr.data()+il);
        getBigEndian(xlNum(offset), tr.data()+xl);
        getBigEndian<int16_t>(1, &tr[ScaleCoord]);
        getBigEndian(int32_t(offset), &tr[SeqFNum]);

        EXPECT_CALL(*mock, writeDOMD(offset, ns, 1U, _)).Times(Exactly(1))
                                                        .WillOnce(check3(tr.data(), SEGSz::getMDSz()));

        File::Param prm(1U);
        File::setPrm(0, File::Meta::il, ilNum(offset), &prm);
        File::setPrm(0, File::Meta::xl, xlNum(offset), &prm);
        File::setPrm(0, File::Meta::tn, offset, &prm);
        file->writeParam(offset, 1U, &prm);
    }

    void initWriteTrHdrCoord(std::pair<size_t, size_t> item, std::pair<int32_t, int32_t> val,
                             int16_t scal, size_t offset, std::vector<uchar> * tr)
    {
        getBigEndian(scal,              &tr->at(ScaleCoord));
        getBigEndian(val.first,         &tr->at(item.first));
        getBigEndian(val.second,        &tr->at(item.second));
        getBigEndian(int32_t(offset),   &tr->at(SeqFNum));
        EXPECT_CALL(*mock, writeDOMD(offset, ns, 1U, _)).Times(Exactly(1))
                                                        .WillOnce(check3(tr->data(), SEGSz::getMDSz()));
    }

    template <bool readPrm = false, bool MOCK = true>
    void readTraceTest(csize_t offset, size_t tn)
    {
        size_t tnRead = (offset + tn > nt && nt > offset ? nt - offset : tn);
        std::vector<uchar> buf;
        if (MOCK)
        {
            if (mock == nullptr)
            {
                std::cerr << "Using Mock when not initialised: LOC: " << __LINE__ << std::endl;
                return;
            }
            if (readPrm)
                buf.resize(tnRead * SEGSz::getDOSz(ns));
            else
                buf.resize(tnRead * SEGSz::getDFSz(ns));
            for (size_t i = 0U; i < tnRead; i++)
            {
                if (readPrm)
                    std::copy(tr.begin() + (offset+i) * SEGSz::getMDSz(), tr.begin() + (offset+i+1) * SEGSz::getMDSz(), buf.begin() + i*SEGSz::getDOSz(ns));
                for (size_t j = 0U; j < ns; j++)
                {
                    float val = offset + i + j;
                    size_t addr = readPrm ? (i*SEGSz::getDOSz(ns)+SEGSz::getMDSz()+j*sizeof(float)) : (i*ns+j)*sizeof(float);
                    getBigEndian(toint(val), &buf[addr]);
                }
            }
            if (readPrm)
                EXPECT_CALL(*mock, readDO(offset, ns, tnRead, _))
                                .Times(Exactly(1)).WillOnce(SetArrayArgument<3>(buf.begin(), buf.end()));
            else
                EXPECT_CALL(*mock, readDODF(offset, ns, tnRead, _))
                            .Times(Exactly(1)).WillOnce(SetArrayArgument<3>(buf.begin(), buf.end()));
        }

        std::vector<float> bufnew(tn * ns);
        File::Param prm(tn);
        file->readTrace(offset, tn, bufnew.data(), (readPrm ? &prm : const_cast<File::Param *>(File::PARAM_NULL)));
        for (size_t i = 0U; i < tnRead; i++)
        {
            if (readPrm && tnRead * ns)
            {
                ASSERT_EQ(ilNum(i+offset), File::getPrm<llint>(i, File::Meta::il, &prm)) << "Trace Number " << i << " offset " << offset;
                ASSERT_EQ(xlNum(i+offset), File::getPrm<llint>(i, File::Meta::xl, &prm)) << "Trace Number " << i << " offset " << offset;

                ASSERT_DOUBLE_EQ(xNum(i+offset), File::getPrm<geom_t>(i, File::Meta::xSrc, &prm));
                ASSERT_DOUBLE_EQ(yNum(i+offset), File::getPrm<geom_t>(i, File::Meta::ySrc, &prm));
            }
            for (size_t j = 0U; j < ns; j++)
                ASSERT_EQ(bufnew[i*ns + j], float(offset + i + j)) << "Trace Number: " << i << " " << j;
        }
    }

    template <bool readPrm = false, bool MOCK = true>
    void readRandomTraceTest(size_t tn, const std::vector<size_t> offset)
    {
        ASSERT_EQ(tn, offset.size());
        std::vector<uchar> buf;
        if (MOCK)
        {
            if (mock == nullptr)
            {
                std::cerr << "Using Mock when not initialised: LOC: " << __LINE__ << std::endl;
                return;
            }
            if (readPrm)
                buf.resize(tn * SEGSz::getDOSz(ns));
            else
                buf.resize(tn * SEGSz::getDFSz(ns));
            for (size_t i = 0U; i < tn; i++)
            {
                if (readPrm && ns*tn)
                    std::copy(tr.begin() + offset[i] * SEGSz::getMDSz(), tr.begin() + (offset[i]+1) * SEGSz::getMDSz(), buf.begin() + i*SEGSz::getDOSz(ns));
                for (size_t j = 0U; j < ns; j++)
                {
                    size_t addr = readPrm ? (i*SEGSz::getDOSz(ns)+SEGSz::getMDSz()+j*sizeof(float)) : (i*ns+j)*sizeof(float);
                    float val = offset[i] + j;
                    getBigEndian(toint(val), &buf[addr]);
                }
            }
            if (readPrm)
                EXPECT_CALL(*mock, readDO(ns, tn, offset.data(), _))
                            .Times(Exactly(1)).WillOnce(SetArrayArgument<3>(buf.begin(), buf.end()));
            else
                EXPECT_CALL(*mock, readDODF(ns, tn, offset.data(), _))
                            .Times(Exactly(1)).WillOnce(SetArrayArgument<3>(buf.begin(), buf.end()));
        }

        std::vector<float> bufnew(tn * ns);
        File::Param prm(tn);
        if (readPrm)
            file->readTrace(tn, offset.data(), bufnew.data(), &prm);
        else
            file->readTrace(tn, offset.data(), bufnew.data());
        for (size_t i = 0U; i < tn; i++)
        {
            if (readPrm && tn * ns)
            {
                ASSERT_EQ(ilNum(offset[i]), File::getPrm<llint>(i, File::Meta::il, &prm)) << "Trace Number " << i << " offset " << offset[i];
                ASSERT_EQ(xlNum(offset[i]), File::getPrm<llint>(i, File::Meta::xl, &prm)) << "Trace Number " << i << " offset " << offset[i];

                ASSERT_DOUBLE_EQ(xNum(offset[i]), File::getPrm<geom_t>(i, File::Meta::xSrc, &prm));
                ASSERT_DOUBLE_EQ(yNum(offset[i]), File::getPrm<geom_t>(i, File::Meta::ySrc, &prm));
            }
            for (size_t j = 0U; j < ns; j++)
                ASSERT_EQ(bufnew[i*ns + j], float(offset[i] + j)) << "Trace Number: " << offset[i] << " " << j;
        }
    }

    void initWriteHeaders(size_t filePos, uchar * md) {
        coord_t src = coord_t(xNum(filePos), yNum(filePos));
        coord_t rcv = coord_t(xNum(filePos), yNum(filePos));
        coord_t cmp = coord_t(xNum(filePos), yNum(filePos));
        grid_t line = grid_t(ilNum(filePos), xlNum(filePos));

        int16_t scale = scalComp(1, calcScale(src));
        scale = scalComp(scale, calcScale(rcv));
        scale = scalComp(scale, calcScale(cmp));

        getBigEndian(scale, &md[ScaleCoord]);
        setCoord(File::Coord::Src, src, scale, md);
        setCoord(File::Coord::Rcv, rcv, scale, md);
        setCoord(File::Coord::CMP, cmp, scale, md);

        setGrid(File::Grid::Line, line, md);
        getBigEndian(int32_t(filePos), &md[SeqFNum]);
    }

    template <bool writePrm = false, bool MOCK = true>
    void writeTraceTest(csize_t offset, csize_t tn)
    {
        std::vector<uchar> buf;
        if (MOCK)
        {
            EXPECT_CALL(*mock, writeHO(_)).Times(Exactly(1));
            if (mock == nullptr)
            {
                std::cerr << "Using Mock when not initialised: LOC: " << __LINE__ << std::endl;
                return;
            }
            if (writePrm)
                buf.resize(tn * SEGSz::getDOSz(ns));
            else
                buf.resize(tn * SEGSz::getDFSz(ns));
            for (size_t i = 0U; i < tn; i++)
            {
                if(writePrm)
                    initWriteHeaders(offset+i, &buf[i*SEGSz::getDOSz(ns)]);
                for (size_t j = 0U; j < ns; j++)
                {
                    size_t addr = writePrm ? (i*SEGSz::getDOSz(ns)+SEGSz::getMDSz()+j*sizeof(float)) : (i*ns + j)*sizeof(float);
                    float val = offset + i + j;
                    getBigEndian(toint(val), &buf[addr]);
                }
            }
            if (writePrm)
                EXPECT_CALL(*mock, writeDO(offset, ns, tn, _))
                                .Times(Exactly(1)).WillOnce(check3(buf.data(), buf.size()));
            else
                EXPECT_CALL(*mock, writeDODF(offset, ns, tn, _))
                                .Times(Exactly(1)).WillOnce(check3(buf.data(), buf.size()));
        }
        File::Param prm(tn);
        std::vector<float> bufnew(tn * ns);
        if (writePrm)
        {
            for (size_t i = 0U; i < tn; i++)
            {
                File::setPrm(i, File::Meta::xSrc, xNum(offset+i), &prm);
                File::setPrm(i, File::Meta::xRcv, xNum(offset+i), &prm);
                File::setPrm(i, File::Meta::xCmp, xNum(offset+i), &prm);
                File::setPrm(i, File::Meta::ySrc, yNum(offset+i), &prm);
                File::setPrm(i, File::Meta::yRcv, yNum(offset+i), &prm);
                File::setPrm(i, File::Meta::yCmp, yNum(offset+i), &prm);
                File::setPrm(i, File::Meta::il, ilNum(offset+i), &prm);
                File::setPrm(i, File::Meta::xl, xlNum(offset+i), &prm);
                File::setPrm(i, File::Meta::tn, offset+i, &prm);
                for (size_t j = 0U; j < ns; j++)
                    bufnew[i*ns + j] = float(offset + i + j);
            }

            file->writeTrace(offset, tn, bufnew.data(), &prm);
        }
        else
        {
            for (size_t i = 0U; i < tn; i++)
                for (size_t j = 0U; j < ns; j++)
                    bufnew[i*ns + j] = float(offset + i + j);

            file->writeTrace(offset, tn, bufnew.data());
        }

        if (MOCK == false)
            readTraceTest<writePrm, MOCK>(offset, tn);
    }

    template <bool writePrm = false, bool MOCK = true>
    void writeRandomTraceTest(size_t tn, const std::vector<size_t> offset)
    {
        ASSERT_EQ(tn, offset.size());
        std::vector<uchar> buf;
        if (MOCK)
        {
            EXPECT_CALL(*mock, writeHO(_)).Times(Exactly(1));
            if (mock == nullptr)
            {
                std::cerr << "Using Mock when not initialised: LOC: " << __LINE__ << std::endl;
                return;
            }
            if (writePrm)
                buf.resize(tn * SEGSz::getDOSz(ns));
            else
                buf.resize(tn * SEGSz::getDFSz(ns));
            for (size_t i = 0U; i < tn; i++)
            {
                if (writePrm)
                    initWriteHeaders(offset[i], &buf[i*SEGSz::getDOSz(ns)]);
                for (size_t j = 0U; j < ns; j++)
                {
                    size_t addr = writePrm ? (i*SEGSz::getDOSz(ns)+SEGSz::getMDSz()+j*sizeof(float)) : (i*ns + j)*sizeof(float);
                    float val = offset[i] + j;
                    getBigEndian(toint(val), &buf[addr]);
                }
            }
            if (writePrm)
                EXPECT_CALL(*mock, writeDO(ns, tn, offset.data(), _))
                                .Times(Exactly(1)).WillOnce(check3(buf.data(), buf.size()));
            else
                EXPECT_CALL(*mock, writeDODF(ns, tn, offset.data(), _))
                                .Times(Exactly(1)).WillOnce(check3(buf.data(), buf.size()));
        }
        File::Param prm(tn);
        std::vector<float> bufnew(tn * ns);
        for (size_t i = 0U; i < tn; i++)
        {
            if (writePrm)
            {
                File::setPrm(i, File::Meta::xSrc, xNum(offset[i]), &prm);
                File::setPrm(i, File::Meta::xRcv, xNum(offset[i]), &prm);
                File::setPrm(i, File::Meta::xCmp, xNum(offset[i]), &prm);
                File::setPrm(i, File::Meta::ySrc, yNum(offset[i]), &prm);
                File::setPrm(i, File::Meta::yRcv, yNum(offset[i]), &prm);
                File::setPrm(i, File::Meta::yCmp, yNum(offset[i]), &prm);
                File::setPrm(i, File::Meta::il, ilNum(offset[i]), &prm);
                File::setPrm(i, File::Meta::xl, xlNum(offset[i]), &prm);
                File::setPrm(i, File::Meta::tn, offset[i], &prm);
            }
            for (size_t j = 0U; j < ns; j++)
                bufnew[i*ns + j] = float(offset[i] + j);
        }

        if (writePrm)
            file->writeTrace(tn, offset.data(), bufnew.data(), &prm);
        else
            file->writeTrace(tn, offset.data(), bufnew.data());

        if (MOCK == false)
            readRandomTraceTest<writePrm, MOCK>(tn, offset);
    }

    template <bool MOCK = true>
    void writeTraceHeaderTest(csize_t offset, csize_t tn)
    {
        std::vector<uchar> buf;
        if (MOCK)
        {
            buf.resize(tn * SEGSz::getMDSz());
            for (size_t i = 0; i < tn; i++)
            {
                coord_t src = coord_t(ilNum(i+1), xlNum(i+5));
                coord_t rcv = coord_t(ilNum(i+2), xlNum(i+6));
                coord_t cmp = coord_t(ilNum(i+3), xlNum(i+7));
                grid_t line = grid_t(ilNum(i+4), xlNum(i+8));

                int16_t scale = scalComp(1, calcScale(src));
                scale = scalComp(scale, calcScale(rcv));
                scale = scalComp(scale, calcScale(cmp));

                uchar * md = &buf[i*SEGSz::getMDSz()];
                getBigEndian(scale, &md[ScaleCoord]);
                setCoord(File::Coord::Src, src, scale, md);
                setCoord(File::Coord::Rcv, rcv, scale, md);
                setCoord(File::Coord::CMP, cmp, scale, md);

                setGrid(File::Grid::Line, line, md);
                getBigEndian(int32_t(offset + i), &md[SeqFNum]);
            }
            EXPECT_CALL(*mock.get(), writeDOMD(offset, ns, tn, _))
                        .Times(Exactly(1)).WillOnce(check3(buf.data(), buf.size()));
        }

        File::Param prm(tn);
        for (size_t i = 0; i < tn; i++)
        {
            File::setPrm(i, File::Meta::xSrc, ilNum(i+1), &prm);
            File::setPrm(i, File::Meta::xRcv, ilNum(i+2), &prm);
            File::setPrm(i, File::Meta::xCmp, ilNum(i+3), &prm);
            File::setPrm(i, File::Meta::il, ilNum(i+4), &prm);
            File::setPrm(i, File::Meta::ySrc, xlNum(i+5), &prm);
            File::setPrm(i, File::Meta::yRcv, xlNum(i+6), &prm);
            File::setPrm(i, File::Meta::yCmp, xlNum(i+7), &prm);
            File::setPrm(i, File::Meta::xl, xlNum(i+8), &prm);
            File::setPrm(i, File::Meta::tn, offset + i, &prm);
        }
        file->writeParam(offset, prm.size(), &prm);
    }
};
typedef FileSEGYTest FileSEGYWrite;
typedef FileSEGYTest FileSEGYRead;
typedef FileSEGYTest FileSEGYIntegRead;
typedef FileSEGYTest FileSEGYIntegWrite;

