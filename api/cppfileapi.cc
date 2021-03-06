/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date December 2016
 *   \brief
 *   \details Functions etc for C++ API
 *//*******************************************************************************************/
#include "cppfileapi.hh"
#include "global.hh"
#include "file/filesegy.hh"
#include "object/objsegy.hh"
#include "data/datampiio.hh"
namespace PIOL {
ExSeis::ExSeis(const Log::Verb maxLevel)
{
    piol = std::make_shared<ExSeisPIOL>(maxLevel);
}

ExSeis::ExSeis(bool initComm, const Log::Verb maxLevel)
{
    piol = std::make_shared<ExSeisPIOL>(initComm, maxLevel);
}

ExSeis::ExSeis(MPI_Comm comm, const Log::Verb maxLevel)
{
    Comm::MPI::Opt copt;
    copt.initMPI = false;
    copt.comm = comm;
    piol = std::make_shared<ExSeisPIOL>(copt, maxLevel);
}

void ExSeis::isErr(std::string msg) const
{
    piol->isErr(msg);
}

namespace File {
ReadDirect::ReadDirect(const Piol piol, const std::string name)
{
    const File::ReadSEGY::Opt f;
    const Obj::SEGY::Opt o;
    const Data::MPIIO::Opt d;
    auto data = std::make_shared<Data::MPIIO>(piol, name, d, FileMode::Read);
    auto obj = std::make_shared<Obj::SEGY>(piol, name, o, data, FileMode::Read);
    file = std::make_shared<File::ReadSEGY>(piol, name, f, obj);
}

WriteDirect::WriteDirect(const Piol piol, const std::string name)
{
    const File::WriteSEGY::Opt f;
    const Obj::SEGY::Opt o;
    const Data::MPIIO::Opt d;
    auto data = std::make_shared<Data::MPIIO>(piol, name, d, FileMode::Write);
    auto obj = std::make_shared<Obj::SEGY>(piol, name, o, data, FileMode::Write);
    file = std::make_shared<File::WriteSEGY>(piol, name, f, obj);
}

const std::string & ReadDirect::readText(void) const
{
    return file->readText();
}

size_t ReadDirect::readNs(void) const
{
    return file->readNs();
}

size_t ReadDirect::readNt(void)
{
    return file->readNt();
}

geom_t ReadDirect::readInc(void) const
{
    return file->readInc();
}

void ReadDirect::readParam(csize_t offset, csize_t sz, Param * prm) const
{
    file->readParam(offset, sz, prm);
}

void WriteDirect::writeParam(csize_t offset, csize_t sz, const Param * prm)
{
    file->writeParam(offset, sz, prm);
}

void ReadDirect::readTrace(csize_t offset, csize_t sz, trace_t * trace, Param * prm) const
{
    file->readTrace(offset, sz, trace, prm);
}

void WriteDirect::writeTrace(csize_t offset, csize_t sz, trace_t * trace, const Param * prm)
{
    file->writeTrace(offset, sz, trace, prm);
}

void ReadDirect::readTrace(csize_t sz, csize_t * offset, trace_t * trace, Param * prm) const
{
    file->readTrace(sz, offset, trace, prm);
}

void ReadDirect::readTraceNonMono(csize_t sz, csize_t * offset, trace_t * trace, Param * prm) const
{
    file->readTraceNonMono(sz, offset, trace, prm);
}

void WriteDirect::writeTrace(csize_t sz, csize_t * offset, trace_t * trace, const Param * prm)
{
    file->writeTrace(sz, offset, trace, prm);
}

void ReadDirect::readParam(csize_t sz, csize_t * offset, Param * prm) const
{
    file->readParam(sz, offset, prm);
}

void WriteDirect::writeParam(csize_t sz, csize_t * offset, const Param * prm)
{
    file->writeParam(sz, offset, prm);
}

void WriteDirect::writeText(const std::string text_)
{
    file->writeText(text_);
}

void WriteDirect::writeNs(const size_t ns_)
{
    file->writeNs(ns_);
}

void WriteDirect::writeNt(const size_t nt_)
{
    file->writeNt(nt_);
}

void WriteDirect::writeInc(const geom_t inc_)
{
    file->writeInc(inc_);
}
}}
