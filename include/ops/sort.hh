/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date November 2016
 *   \brief
*//*******************************************************************************************/
#ifndef PIOLOPSSORT_INCLUDE_GUARD
#define PIOLOPSSORT_INCLUDE_GUARD
#include "global.hh"
#include "share/param.hh"

namespace PIOL { namespace File {
/*! A template for the Compare less-than function
 */
template <class T>
using Compare = std::function<bool(const T &, const T &)>;

/*! Function to sort the metadata in a Param struct. The returned vector is the location where the nth parameter
 *  is located in the sorted list. Implementation note: the Param vector is used internally
 *  to allow random-access iterator support.
 *  \param[in] piol The PIOL object.
 *  \param[in,out] prm The parameter structure to sort
 *  \param[in] comp The Param function to use for less-than comparisons between objects in the
 *                  vector. It assumes each Param structure has exactly one entry.
 *  \param[in] FileOrder Do we wish to have the sort in the sorted input order (true) or sorted order (false)
 *  \return Return the correct order of traces from those which are smallest with respect to the comp function.
 */
std::vector<size_t> sort(ExSeisPIOL * piol, Param * prm, Compare<Param> comp, bool FileOrder = true);

/*! Check that the file obeys the expected ordering.
 *  \param[in] src The input file.
 *  \param[in] dec The decomposition: a pair which contains the offset (first) and the number of traces for the local process.
 *  \return Return true if the local ordering is correct.
 */
extern bool checkOrder(ReadInterface * src, std::pair<size_t, size_t> dec);
}}
#endif
