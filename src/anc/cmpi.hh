/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date July 2016
 *   \brief The MPI communicator
 *   \details The MPI communicator class is responsible for the concrete implementation of
 *   all MPI communication specific features. That is, everything within the MPI specification
 *   except those features which are related to MPI-IO or overlap with MPI-IO.
*//*******************************************************************************************/
#ifndef PIOLCOMMMPI_INCLUDE_GUARD
#define PIOLCOMMMPI_INCLUDE_GUARD
#include "global.hh"
#include "anc/comm.hh"
#include <mpi.h>

namespace PIOL { namespace Comm {
/*! \brief The MPI-Communicator options structure.
 */
struct MPIOpt : Opt
{
    MPI_Comm comm = MPI_COMM_WORLD; //!< This variable defines the default MPI communicator.
    bool initMPI = true;            //!< If \c initMPI is true, MPI initialisation is performed. Otherwise it is skipped.
};

/*! \brief The MPI communication class. All MPI communication specific routines should be wrapped up and accessible from this class.
 */
class MPI : public Comm::Interface
{
    private :
    MPI_Comm comm;  //!< The MPI communicator.
    bool init;      //!< This variable records whether the class is responsible for initialisation of MPI or not.
    public :

    /*! \brief The constructor.
     *  \param[in] opt Any options for the communication layer.
     */
    MPI(const MPIOpt & opt);
    /*! \brief The destructor. If the object is responsible for initialisation it deinitialises MPI
     *  in this routine.
     */
    ~MPI(void);
    /*! \brief Retrieve the MPI communicator associated with the ExSeisPIOL.
     *  \return The MPI communicator.
     */
    MPI_Comm getComm();
    /*! \brief All processors will wait at the barrier until everyone arrives.
     *  The MPI implementation of the pure virtual base member simply calls MPI_Barrier
     */
    void barrier(void);
};
}}
#endif
