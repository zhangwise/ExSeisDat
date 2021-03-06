/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date July 2016
 *   \brief
 *   \details
 *//*******************************************************************************************/
#include "object/object.hh"
#include "data/data.hh"
namespace PIOL { namespace Obj {
size_t Interface::getFileSz(void) const
{
    return data->getFileSz();
}

void Interface::setFileSz(csize_t sz) const
{
    return data->setFileSz(sz);
}
}}
