/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date November 2016
 *   \brief The Sort Operation
 *   \details The algorithm used is a nearest neighbour approach where at each iteration
 *   the lowest valued metadata entries are moved to adjacent processes with a lower rank and a
 *   sort is performed. After the sort the highest entries are moved again to the process with a
 *   higher rank. If each process has the same traces it started off with, the sort
 *   is complete.
*//*******************************************************************************************/
#include <algorithm>
#include <iterator>
#include <functional>

#include "global.hh"
#include "fileops.hh"
#include "ops/minmax.hh"

#include <cstring>
namespace PIOL { namespace File {
void getMinMax(ExSeisPIOL * piol, size_t offset, size_t sz, const coord_t * coord, CoordElem * minmax)
{
    auto xlam = [](const coord_t & a) -> geom_t { return a.x; };
    auto ylam = [](const coord_t & a) -> geom_t { return a.y; };
    getMinMax<coord_t>(piol, offset, sz, coord, xlam, ylam, minmax);
}

void getMinMax(ExSeisPIOL * piol, size_t offset, size_t sz, File::Coord item, const TraceParam * prm, CoordElem * minmax)
{
    switch (item)
    {
        case File::Coord::Src :
        getMinMax<TraceParam>(piol, offset, sz, prm,
                          [](const TraceParam & a) -> geom_t { return a.src.x; },
                          [](const TraceParam & a) -> geom_t { return a.src.y; },
                          minmax);
        break;
        case File::Coord::Rcv :
        getMinMax<TraceParam>(piol, offset, sz, prm,
                          [](const TraceParam & a) -> geom_t { return a.rcv.x; },
                          [](const TraceParam & a) -> geom_t { return a.rcv.y; },
                          minmax);
        break;
        case File::Coord::CMP :
        getMinMax<TraceParam>(piol, offset, sz, prm,
                          [](const TraceParam & a) -> geom_t { return a.cmp.x; },
                          [](const TraceParam & a) -> geom_t { return a.cmp.y; },
                          minmax);
        break;
        default :
            piol->log->record("", Log::Layer::Ops, Log::Status::Warning,
                         "getCoordMinMax() was called for an unknown coordinate.",
                          Log::Verb::Extended);
        break;
    }
}
}}