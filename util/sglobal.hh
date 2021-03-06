#ifndef UTIL_SGLOBALHH_INCLUDE_GUARD
#define UTIL_SGLOBALHH_INCLUDE_GUARD
#define _POSIX_C_SOURCE 200809L //POSIX includes
#include <unistd.h>
#include <utility>
#include "global.hh"

/*! Function which takes the size of a problem, the number of ranks to
 *  decompose the problem over and the particular local rank for the function call.
 *  \param[in] sz The size of the problem.
 *  \param[in] numRank The total number of ranks.
 *  \param[in] rank The rank during the function call, generally the rank of the calling
 *             MPI process.
 *  \return Return an extent, a starting point (generally for a 'for' loop) and the number
 *          of iterations.
 */
extern std::pair<size_t, size_t> decompose(size_t sz, size_t numRank, size_t rank);

/*! Decomposition which takes account that the data may come in logical blocks
 *  \param[in] sz The size of the problem.
 *  \param[in] bsz The block size
 *  \param[in] numRank The total number of ranks.
 *  \param[in] rank The rank during the function call, generally the rank of the calling
 *             MPI process.
 *  \return Return an extent, a starting point (generally for a 'for' loop) and the number
 *          of iterations.
 */
extern std::pair<size_t, size_t> blockDecomp(size_t sz, size_t bsz, size_t numRank, size_t rank, size_t off = 0);

/*! Unequal decomposition, weights towards the lower ranks. Useful for testing purposes
 *  \param[in] sz The size of the problem.
 *  \param[in] numRank The total number of ranks.
 *  \param[in] rank The rank during the function call, generally the rank of the calling
 *             MPI process.
 *  \return Return an extent, a starting point (generally for a 'for' loop) and the number
 *          of iterations.
 */
extern std::vector<size_t> lobdecompose(PIOL::ExSeisPIOL * piol, size_t sz, size_t numRank, size_t rank);

#endif
