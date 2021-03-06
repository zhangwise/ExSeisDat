#include "sglobal.hh"
#include <assert.h>
#include <memory>
#include <string>
#include <iostream>
#include <cmath>
#include "cppfileapi.hh"
#include "share/segy.hh"
using namespace PIOL;

void writeContig(ExSeis piol, File::WriteDirect * file, size_t offset, size_t nt, size_t ns, size_t lnt, size_t extra, size_t max)
{
    float fhalf = float(nt*ns)/2.0;
    float off = float(nt*ns)/4.0;
    long nhalf = nt/2;
    File::Param prm(max);
    std::vector<float> trc(max*ns);
    for (size_t i = 0; i < lnt; i += max)
    {
        size_t rblock = (i + max < lnt ? max : lnt - i);
        for (size_t j = 0; j < rblock; j++)
        {
            float k = nhalf - std::abs(-nhalf + long(offset+i+j));
            setPrm(j, Meta::xSrc, 1600.0 + k, &prm);
            setPrm(j, Meta::ySrc, 2400.0 + k, &prm);
            setPrm(j, Meta::xRcv, 100000.0 + k, &prm);
            setPrm(j, Meta::yRcv, 3000000.0 + k, &prm);
            setPrm(j, Meta::xCmp, 10000.0 + k, &prm);
            setPrm(j, Meta::yCmp, 4000.0 + k, &prm);
            setPrm(j, Meta::il, 2400 + k, &prm);
            setPrm(j, Meta::xl, 1600 + k, &prm);
            setPrm(j, Meta::tn, offset+i+j, &prm);
        }
        for (size_t j = 0; j < trc.size(); j++)
            trc[j] = fhalf - std::abs(-fhalf + float((offset+i)*ns+j)) - off;
        file->writeTrace(offset + i, rblock, trc.data(), &prm);
        piol.isErr();
    }
    for (size_t j = 0; j < extra; j++)
    {
        file->writeTrace(0U, size_t(0), nullptr, File::PARAM_NULL);
        piol.isErr();
    }
}

void writeRandom(ExSeisPIOL * piol, File::WriteDirect * file, size_t nt, size_t ns, size_t lnt, size_t extra, size_t max)
{
    float fhalf = float(nt*ns)/2.0;
    float off = float(nt*ns)/4.0;
    long nhalf = nt/2;

    std::vector<size_t> offset(lnt);
    auto num = piol->comm->gather<size_t>(lnt);

    size_t rank = piol->comm->getRank();
    size_t numRank = piol->comm->getNumRank();

    size_t offcount = 0;
    size_t t = 0;

    //Work out offsets
    while (num[rank] > 0 && offcount < nt)
    {
        for (size_t j = 0; j < numRank; j++)
            if (j != rank)
            {
                if (num[j] != 0)
                {
                    num[j]--;
                    offcount++;
                }
            }
            else
            {
                num[rank]--;
                offset[t++] = offcount++;
            }
    }

    for (size_t i = 0; i < lnt; i += max)
    {
        size_t rblock = (i + max < lnt ? max : lnt - i);
        File::Param prm(rblock);
        std::vector<float> trc(rblock*ns);

        for (size_t j = 0; j < rblock; j++)
        {
            float k = nhalf - std::abs(-nhalf + long(offset[i+j]));
            setPrm(j, Meta::xSrc, 1600.0 + k, &prm);
            setPrm(j, Meta::ySrc, 2400.0 + k, &prm);
            setPrm(j, Meta::xRcv, 100000.0 + k, &prm);
            setPrm(j, Meta::yRcv, 3000000.0 + k, &prm);
            setPrm(j, Meta::xCmp, 10000.0 + k, &prm);
            setPrm(j, Meta::yCmp, 4000.0 + k, &prm);
            setPrm(j, Meta::il, 2400 + k, &prm);
            setPrm(j, Meta::xl, 1600 + k, &prm);
            setPrm(j, Meta::tn, offset[i+j], &prm);
        }
        for (size_t j = 0; j < trc.size(); j++)
            trc[j] = fhalf - std::abs(-fhalf + float((offset[i])*ns+j)) - off;
        file->writeTrace(rblock, &offset[i], trc.data(), &prm);
        piol->isErr();
    }
    for (size_t j = 0; j < extra; j++)
    {
        file->writeTrace(0U, (size_t *)NULL, nullptr, File::PARAM_NULL);
        piol->isErr();
    }
}

void FileMake(bool lob, bool random, const std::string name, size_t max, size_t ns, size_t nt, geom_t inc)
{
    ExSeis piol;
    File::WriteDirect file(piol, name);

    piol.isErr();
    file.writeNs(ns);
    file.writeNt(nt);
    file.writeInc(inc);
    file.writeText("Test file\n");
    piol.isErr();

    size_t offset = 0;
    size_t lnt = 0;
    size_t biggest = 0;

    if (lob)
    {
        auto dec = lobdecompose(piol, nt, piol.getNumRank(), piol.getRank());
        offset = dec[0];
        lnt = dec[1];
        biggest = dec[2];
    }
    else
    {
        auto dec = decompose(nt, piol.getNumRank(), piol.getRank());
        offset = dec.first;
        lnt = dec.second;
        ExSeisPIOL * tpiol = piol;
        biggest = tpiol->comm->max(lnt);
    }

    //TODO: Add memusage for Param
    max /= (SEGSz::getDOSz(ns) + SEGSz::getDFSz(ns)  + sizeof(size_t));
    size_t extra = biggest/max - lnt/max + (biggest % max > 0) - (lnt % max > 0);
    if (random)
        writeRandom(piol, &file, nt, ns, lnt, extra, max);
    else
        writeContig(piol, &file, offset, nt, ns, lnt, extra, max);
}

int main(int argc, char ** argv)
{
    std::string name = "";
    size_t ns = 0;
    size_t nt = 0;
    size_t max = 0;
    geom_t inc = 0.0;
    bool lob = false;
    bool random = false;

    if (argc <= 1)
    {
        std::cout << "Options: filemake -o \"name.segy\" -s <ns> -t <nt> -m <mem(MiB)> -i <inc>\n";
        return EXIT_FAILURE;
    }
    //TODO document these arguments
    // o output file
    // s number of samples
    // t number of traces
    // m max
    // i increment
    // l lobsided
    // r 'random' offsets
    std::string opt = "s:t:m:o:i:lr";  //TODO: uses a GNU extension
    for (int c = getopt(argc, argv, opt.c_str()); c != -1; c = getopt(argc, argv, opt.c_str()))
        switch (c)
        {
            case 'o' :
                name = optarg;
            break;
            case 's' :
                ns = std::stoul(optarg);
            break;
            case 't' :
                nt = std::stoul(optarg);
            break;
            case 'm' :
                max = std::stoul(optarg);
            break;
            case 'i' :
                inc = std::stod(optarg);
            break;
            case 'l' :
                lob = true;
            break;
            case 'r' :
                random = true;
            break;
            default :
                std::cerr << "One of the command line arguments is invalid\n";
            break;
        }

    assert(name.size() && max && inc != 0.0);
    max *= 1024 * 1024;
    FileMake(lob, random, name, max, ns, nt, inc);
    return 0;
}

