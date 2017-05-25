/************************************************************************ *******************//*!
 *   \file
 *   \author Meghan Fisher - meghan.fisher@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date May 2017
 *   \brief The Automatic Gain Control Operation
 *   \details The algorithm finds some type of moving average (RMS, RMS with Triangle window,
 *   Mean Abs, and Median) to trace amplitudes for visualization purposes. It can be applied to
 *   traces independantly or applies the same scalar to all traces at the same height.
*//*******************************************************************************************/
#include <vector>
#include <algorithm>
#include <cmath>
#include <assert.h>
#include "global.hh"
#include "ops/agc.hh"
#include "share/api.hh"
namespace PIOL { namespace File {

trace_t AGCRMS(trace_t * trc, size_t window, trace_t normR, llint winCntr)
{
    trace_t amp = trace_t(0);
    for (size_t j = 0U; j < window; j++)
        amp += trc[j] * trc[j];

    size_t num = std::count_if(trc, &trc[window], [](trace_t i){ return i != trace_t(0); });

    return normR / std::sqrt(amp / (!num ? 1U : num));
}

trace_t AGCRMSTri(trace_t * trc, size_t window, trace_t normR, llint winCntr)
{
    trace_t amp = trace_t(0);
    trace_t winFullTail = std::max(fabs(winCntr), fabs(window - winCntr - trace_t(1)));

    for (llint j = 0U; j < window; j++)
        amp += pow(trc[j] * (trace_t(1) - std::abs(trace_t(j-winCntr)/winFullTail)), trace_t(2));

    size_t num = std::count_if(trc, &trc[window], [](trace_t i){ return i != trace_t(0); });

    return normR / std::sqrt(amp / (!num ? 1U : num));
}

trace_t AGCMeanAbs(trace_t * trc, size_t window, trace_t normR, llint winCntr)
{
    trace_t amp = trace_t(0);
    for (size_t j = 0U; j < window; j++)
        amp += trc[j];
    size_t num = std::count_if(trc, &trc[window], [](trace_t i){ return i != trace_t(0); });
    return normR/(std::abs(amp)/(!num ? 1U : num));
}

trace_t AGCMedian(trace_t * trc, size_t window, trace_t normR, llint winCntr)
{
    std::vector<trace_t> trcTmp(trc, &trc[window]);
    std::sort(trcTmp.begin(), trcTmp.end());
    return normR/(window % 2U == 0U ? ((trcTmp[window/2U]+trcTmp[window/2U+1U])/trace_t(2)) : trcTmp[window/2U]);;
}

void AGC(csize_t nt, csize_t ns, trace_t * trc, const AGCFunc func, size_t window, trace_t normR)
{
    window = (window % 2U == 0 ? window + 1U : window);
    assert(ns > window);
    std::vector<trace_t> trcAGC(ns);
    size_t win2 = window/2U;
    for (size_t i = 0; i < nt; i++)
    {
        for (size_t j = 0;  j < win2 + 1U; j++)
        {
            trcAGC[j] = func(&trc[i*ns], win2+j+1U, normR, j);
            trcAGC[j + ns - win2] = func(&trc[(i+1U)*ns - 2U*win2 + j],  2U*win2 - j, normR, win2);
        }

        for (size_t j = win2 + 1U; j < ns - win2; j++)
            trcAGC[j] = func(&trc[i*ns+j - win2], window, normR, win2);

        for (size_t j = 0; j < ns; j++)
            trc[i*ns+j] *= trcAGC[j];
    }
}

AGCFunc getAGCFunc(AGCType type)
{
    switch (type)
    {
        default :
        case AGCType::RMS :
            return AGCRMS;
        case AGCType::RMSTri :
            return AGCRMSTri;
        case AGCType::MeanAbs :
            return AGCMeanAbs;
        case AGCType::Median :
            return AGCMedian;
    }
}
}}
