/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date May 2017
 *   \brief
 *   \details
 *//*******************************************************************************************/
#include "set/cache.hh"
#include "share/api.hh"
#include "file/dynsegymd.hh"
namespace PIOL {
#warning TODO: can this be made to handle optionally prm and trc?
File::Param * Cache::cachePrm(std::shared_ptr<File::Rule> rule, FileDeque & desc)
{
    auto it = std::find_if(cache.begin(), cache.end(), [desc] (const CacheElem & elem) -> bool { return elem.desc == desc; });

    if (it == cache.end() || !it->prm)
    {
        size_t lsnt = 0U;
        for (auto & f : desc)
            lsnt += f->olst.size();
        size_t off = piol->comm->offset(lsnt);

        size_t nt = 0U;
        for (auto & f : desc)
            nt += f->ifc->readNt();

        auto prm = std::make_unique<File::Param>(rule, lsnt);

        //TODO: Do not make assumptions about Parameter sizes fitting in memory.
        size_t loff = 0;
        size_t c = 0;
        for (auto & f : desc)
        {
            f->ifc->readParam(f->ilst.size(), f->ilst.data(), prm.get(), loff);
            for (size_t i = 0; i < f->ilst.size(); i++)
            {
                File::setPrm(loff+i, Meta::gtn, off + loff + i, prm.get());
                File::setPrm(loff+i, Meta::ltn, f->ilst[i] * desc.size() + c, prm.get());
            }
            c++;
            loff += f->ilst.size();
        }

        if (it == cache.end())
        {
            cache.emplace_back(desc, std::move(prm));
            it = cache.end() - 1U;
        }
        else
            it->prm = std::move(prm);
    }
    else
    {
#warning Check if prm is cached and same rules
    }
    return it->prm.get();
}
}
