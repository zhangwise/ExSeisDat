/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date July 2016
 *   \brief
 *   \details
 *//*******************************************************************************************/
#include "anc/log.hh"
#include <iostream>
namespace PIOL { namespace Log {
void Logger::record(const std::string file, const Layer layer, const Status stat, const std::string msg, const Verb verbosity, bool condition)
{
    if (condition)
    {
        if (static_cast<size_t>(verbosity) <= static_cast<size_t>(maxLevel))
            loglist.push_front({file, layer, stat, msg, verbosity});

        if (stat == Status::Error)
            error = true;
    }
}

size_t Logger::numStat(const Status stat) const
{
    size_t sz = 0;
    for (auto item : loglist)
        sz += (item.stat == stat); //The spec guarantees this is one if the equality holds
    return sz;
}

bool Logger::isErr(void) const
{
    return error;
}

void Logger::procLog(void)
{
    while (!loglist.empty())
    {
        Item & item = loglist.front();

        //Although this currently prints to standard error, the point of this logging mechanism is that eventually this
        //could be changed to output to file or hook into some GUI etc.
        std::cerr << item.file << " " << static_cast<size_t>(item.layer) << " " << static_cast<size_t>(item.stat) <<
                     " " << item.msg << " " << static_cast<size_t>(item.vrbsy) << std::endl;
        loglist.pop_front();
    }
}
}}
