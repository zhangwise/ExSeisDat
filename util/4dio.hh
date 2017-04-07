/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \date January 2017
 *   \brief
 *   \details
 *//*******************************************************************************************/

#ifndef FOURDBIN4DIO_INCLUDE_GUARD
#define FOURDBIN4DIO_INCLUDE_GUARD
#include <limits>
#include "4d.hh"
#include "share/segy.hh"

namespace PIOL { namespace FOURD {
/*! This structure is for holding ALIGN aligned memory containing the coordinates.
 */
struct Coords
{
    size_t sz;              //!< The number of sets of coordinates
    fourd_t * xSrc = NULL;  //!< The x src coordinates
    fourd_t * ySrc = NULL;  //!< The y src coordinates
    fourd_t * xRcv = NULL;  //!< The x rcv coordinates
    fourd_t * yRcv = NULL;  //!< The y rcv coordinates
    size_t * tn = NULL;     //!< The trace number
    size_t allocSz;         //!< The size which was actually allocated

    llint * il = NULL;  //!< The y rcv coordinates
    llint * xl = NULL;  //!< The y rcv coordinates

    /*! Constructor for coords. Allocate each array to take sz_ entries
     *  but also make sure that the allocated space is aligned.
     *  \param[in] sz_ Number of traces
     */
    Coords(size_t sz_) : sz(sz_)
    {
        allocSz = ((sz + ALIGN) / ALIGN) * ALIGN;

        //posix_memalign() guarantees the memory allocated is alligned according to the alignment
        //value
        posix_memalign(reinterpret_cast<void **>(&xSrc), ALIGN, allocSz * sizeof(fourd_t));
        posix_memalign(reinterpret_cast<void **>(&ySrc), ALIGN, allocSz * sizeof(fourd_t));
        posix_memalign(reinterpret_cast<void **>(&xRcv), ALIGN, allocSz * sizeof(fourd_t));
        posix_memalign(reinterpret_cast<void **>(&yRcv), ALIGN, allocSz * sizeof(fourd_t));
        posix_memalign(reinterpret_cast<void **>(&tn), ALIGN, allocSz * sizeof(size_t));

        posix_memalign(reinterpret_cast<void **>(&il), ALIGN, allocSz * sizeof(llint));
        posix_memalign(reinterpret_cast<void **>(&xl), ALIGN, allocSz * sizeof(llint));

        for (size_t i = 0; i < allocSz; i++)
        {
            xSrc[i] = ySrc[i] = xRcv[i] = yRcv[i] = std::numeric_limits<fourd_t>::max();
            il[i] = xl[i] == std::numeric_limits<llint>::max();
        }
    }

    /*! Destructor. Deallocate all the memory.
     */
    ~Coords(void)
    {
        if (xSrc)
            free(xSrc);
        if (ySrc)
            free(ySrc);
        if (xRcv)
            free(xRcv);
        if (yRcv)
            free(yRcv);
        if (tn)
            free(tn);
    }
};

/*! Given an input file, perform a decomposition over the traces and return a subset of the coordinates to
 *  each process. The coordinates returned have been sorted into xSrc order.
 *  \param[in] piol The piol object (in a shared pointer).
 *  \param[in] name The name of the input file.
 *  \return Return a unique_ptr to the structure containing the coordinates read by the local process.
 */
extern std::unique_ptr<Coords> getCoords(Piol piol, std::string name);

/*! Extract traces and coordinates from an input file \c sname according to what traces are listed in \c list.
 *  \param[in] piol The piol object (in a shared pointer).
 *  \param[in] dname The name of the destination file.
 *  \param[in] sname The name of the source file.
 *  \param[in] list The list of traces to read from the input file in the order they should appear in the output file.
 *  \param[in] minrs The value of minrs which should be stored with the trace header of each trace.
 */
extern void outputNonMono(Piol piol, std::string dname, std::string sname, vec<size_t> & list, vec<fourd_t> & minrs, bool printDsr);
}}
#endif
