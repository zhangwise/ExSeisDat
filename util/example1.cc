#include "sglobal.hh"
#include <memory>
#include <string>
#include <iostream>
#include <unistd.h>
#include <assert.h>
#include "cppfileapi.hh"
using namespace PIOL;
int main(int argc, char ** argv)
{
    std::string opt = "o:";  //TODO: uses a GNU extension
    std::string name = "";
    for (int c = getopt(argc, argv, opt.c_str()); c != -1; c = getopt(argc, argv, opt.c_str()))
        if (c == 'o')
            name = optarg;
        else
        {
            std::cerr << "One of the command line arguments is invalid" << std::endl;
            return -1;
        }
    assert(name.size() > 0);

    //Initialise the PIOL by creating an ExSeisPIOL object
    ExSeis piol;

    //Create a SEGY file object
    File::Direct file(piol, name, FileMode::Write);

    //lnt is the number of traces and sets of trace parameters we will write per process
    size_t nt = 40000, ns = 300;
    double inc = 0.04;

    auto dec = decompose(nt, piol.getNumRank(), piol.getRank());
    size_t offset = dec.first;
    size_t lnt = dec.second;

    //Write some header parameters
    file.writeNs(ns);
    file.writeNt(nt);
    file.writeInc(inc);
    file.writeText("Test file\n");

    //Set and write some trace parameters
    File::Param prm(lnt);
    for (size_t j = 0; j < lnt; j++)
    {
        float k = offset+j;
        setPrm(j, File::Meta::xSrc, 1600.0 + k, &prm);
        setPrm(j, File::Meta::ySrc, 2400.0 + k, &prm);
        setPrm(j, File::Meta::xRcv, 100000.0 + k, &prm);
        setPrm(j, File::Meta::yRcv, 3000000.0 + k, &prm);
        setPrm(j, File::Meta::xCmp, 10000.0 + k, &prm);
        setPrm(j, File::Meta::yCmp, 4000.0 + k, &prm);
        setPrm(j, File::Meta::il, 2400 + k, &prm);
        setPrm(j, File::Meta::xl, 1600 + k, &prm);
        setPrm(j, File::Meta::tn, offset+j, &prm);
    }
    file.writeParam(offset, lnt, &prm);

    //Set and write some traces
    std::vector<float> trc(lnt*ns);
    for (size_t j = 0; j < lnt*ns; j++)
        trc[j] = float(offset*ns+j);
    file.writeTrace(offset, lnt, trc.data());
    return 0;
}

