/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date January 2017
 *   \brief The File layer interface
 *   \details The File layer interface is a base class which specific File implementations
 *   work off
*//*******************************************************************************************/
#ifndef PIOLFILE_INCLUDE_GUARD
#define PIOLFILE_INCLUDE_GUARD
#include "global.hh"
#include "share/param.hh"

namespace PIOL { namespace File {
extern const Param * PARAM_NULL;    //!< The NULL parameter so that the correct internal read pattern is selected
/*! \brief The File layer interface. Specific File implementations
 *  work off this base class.
 */
class ReadInterface
{
    protected :
    Piol piol;                            //!< The PIOL object.
    std::string name;                     //!< Store the file name for debugging purposes.
    std::shared_ptr<Obj::Interface> obj;  //!< Pointer to the Object-layer object (polymorphic).
    size_t ns;                            //!< The number of samples per trace.
    size_t nt;                            //!< The number of traces.
    std::string text;                     //!< Human readable text extracted from the file
    geom_t inc;                           //!< The increment between samples in a trace

    public :
    /*! \brief The constructor.
     *  \param[in] piol_ This PIOL ptr is not modified but is used to instantiate another shared_ptr.
     *  \param[in] name_ The name of the file associated with the instantiation.
     *  \param[in] obj_  Pointer to the Object-layer object (polymorphic).
     */
    ReadInterface(const Piol piol_, const std::string name_, std::shared_ptr<Obj::Interface> obj_) : piol(piol_), name(name_), obj(obj_)
    {
    }

    /*! Empty constructor
     */
    ReadInterface(void) { }

    /*! \brief A virtual destructor to allow deletion, unique_ptr polymorphism.
     */
    virtual ~ReadInterface(void) { }

    /*! \brief Read the human readable text from the file
     *  \return A string containing the text (in ASCII format)
     */
    virtual const std::string & readText(void) const;

    /*! \brief Read the name of the file
     *  \return A string containing the name (in ASCII format)
     */
    const std::string & readName(void) const
    {
        return name;
    }

    /*! \brief Read the number of samples per trace
     *  \return The number of samples per trace
     */
    virtual size_t readNs(void) const;

    /*! \brief Read the number of traces in the file
     *  \return The number of traces
     */
    virtual size_t readNt(void) = 0;

    /*! \brief Read the number of increment between trace samples
     *  \return The increment between trace samples
     */
    virtual geom_t readInc(void) const;

    /*! \brief Read the trace parameters from offset to offset+sz of the respective
     *  trace headers.
     *  \param[in] offset The starting trace number.
     *  \param[in] sz The number of traces to process.
     *  \param[in] prm An array of the parameter structures (size sizeof(Param)*sz)
     *  \param[in] skip When reading, skip the first "skip" entries of prm
     */
    virtual void readParam(csize_t offset, csize_t sz, Param * prm, csize_t skip = 0) const = 0;

    /*! \brief Read the traces from offset to offset+sz
     *  \param[in] offset The starting trace number.
     *  \param[in] sz The number of traces to process.
     *  \param[out] trace The array of traces to fill from the file
     *  \param[out] prm A contiguous array of the parameter structures (size sizeof(Param)*sz)
     *  \param[in] skip When reading, skip the first "skip" entries of prm
     */
    virtual void readTrace(csize_t offset, csize_t sz, trace_t * trace, Param * prm = const_cast<Param *>(PARAM_NULL), csize_t skip = 0) const = 0;

    /*! \brief Read the traces specified by the offsets in the passed offset array. Assumes Monotonic.
     *  \param[in] sz The number of traces to process
     *  \param[in] offset An array of trace numbers to read (monotonic list).
     *  \param[out] trace A contiguous array of each trace (size sz*ns*sizeof(trace_t))
     *  \param[out] prm A parameter structure
     *  \param[in] skip When reading, skip the first "skip" entries of prm
     *
     *  \details When prm==PARAM_NULL only the trace DF is read.
     */
    virtual void readTrace(csize_t sz, csize_t * offset, trace_t * trace, Param * prm = const_cast<Param *>(PARAM_NULL), csize_t skip = 0) const = 0;

    /*! \brief Read the traces specified by the offsets in the passed offset array. Does not assume monotonic
     *  \param[in] sz The number of traces to process
     *  \param[in] offset An array of trace numbers to read (non-monotonic list).
     *  \param[out] trace A contiguous array of each trace (size sz*ns*sizeof(trace_t))
     *  \param[out] prm A parameter structure
     *  \param[in] skip When reading, skip the first "skip" entries of prm
     *
     *  \details When prm==PARAM_NULL only the trace DF is read.
     */
    virtual void readTraceNonMono(csize_t sz, csize_t * offset, trace_t * trace, Param * prm = const_cast<Param *>(PARAM_NULL), csize_t skip = 0) const = 0;

    /*! \brief Read the traces specified by the offsets in the passed offset array.
     *  \param[in] sz The number of traces to process
     *  \param[in] offset An array of trace numbers to read.
     *  \param[out] prm A parameter structure
     *  \param[in] skip When reading, skip the first "skip" entries of prm
     */
    virtual void readParam(csize_t sz, csize_t * offset, Param * prm, csize_t skip = 0) const = 0;
};

/*! \brief The File layer interface. Specific File implementations
 *  work off this base class.
 */
class WriteInterface
{
    protected :
    Piol piol;                            //!< The PIOL object.
    std::string name;                     //!< Store the file name for debugging purposes.
    std::shared_ptr<Obj::Interface> obj;  //!< Pointer to the Object-layer object (polymorphic).
    size_t ns;                            //!< The number of samples per trace.
    size_t nt;                            //!< The number of traces.
    std::string text;                     //!< Human readable text extracted from the file
    geom_t inc;                           //!< The increment between samples in a trace

    public :
    /*! \brief The constructor.
     *  \param[in] piol_ This PIOL ptr is not modified but is used to instantiate another shared_ptr.
     *  \param[in] name_ The name of the file associated with the instantiation.
     *  \param[in] obj_  Pointer to the Object-layer object (polymorphic).
     */
    WriteInterface(const Piol piol_, const std::string name_, std::shared_ptr<Obj::Interface> obj_) : piol(piol_), name(name_), obj(obj_)
    {
    }

    /*! Empty constructor
     */
    WriteInterface(void) { }

    /*! \brief A virtual destructor to allow deletion, unique_ptr polymorphism.
     */
    virtual ~WriteInterface(void) { }

    /*! \brief Write the human readable text from the file.
     *  \param[in] text_ The new string containing the text (in ASCII format).
     */
    virtual void writeText(const std::string text_) = 0;

    /*! \brief Write the number of samples per trace
     *  \param[in] ns_ The new number of samples per trace.
     */
    virtual void writeNs(csize_t ns_) = 0;

    /*! \brief Write the number of traces in the file
     *  \param[in] nt_ The new number of traces.
     */
    virtual void writeNt(csize_t nt_) = 0;

    /*! \brief Write the number of increment between trace samples.
     *  \param[in] inc_ The new increment between trace samples.
     */
    virtual void writeInc(const geom_t inc_) = 0;

    /*! \brief Write the trace parameters from offset to offset+sz to the respective
     *  trace headers.
     *  \param[in] offset The starting trace number.
     *  \param[in] sz The number of traces to process.
     *  \param[in] prm An array of the parameter structures (size sizeof(Param)*sz)
     *  \param[in] skip When writing, skip the first "skip" entries of prm
     *
     *  \details It is assumed that this operation is not an update. Any previous
     *  contents of the trace header will be overwritten.
     */
    virtual void writeParam(csize_t offset, csize_t sz, const Param * prm, csize_t skip = 0) = 0;

    /*! \brief Write the traces from offset to offset+sz
     *  \param[in] offset The starting trace number.
     *  \param[in] sz The number of traces to process.
     *  \param[in] trace The array of traces to write to the file
     *  \param[in] prm A contiguous array of the parameter structures (size sizeof(Param)*sz)
     *  \param[in] skip When writing, skip the first "skip" entries of prm
     */
    virtual void writeTrace(csize_t offset, csize_t sz, trace_t * trace, const Param * prm = PARAM_NULL, csize_t skip = 0) = 0;

    /*! \brief Write the traces specified by the offsets in the passed offset array.
     *  \param[in] sz The number of traces to process
     *  \param[in] offset An array of trace numbers to write.
     *  \param[in] trace A contiguous array of each trace (size sz*ns*sizeof(trace_t))
     *  \param[in] prm A parameter structure
     *  \param[in] skip When writing, skip the first "skip" entries of prm
     *
     *  \details When prm==PARAM_NULL only the trace DF is written.
     *  It is assumed that the parameter writing operation is not an update. Any previous
     *  contents of the trace header will be overwritten.
     */
    virtual void writeTrace(csize_t sz, csize_t * offset, trace_t * trace, const Param * prm = PARAM_NULL, csize_t skip = 0) = 0;

    /*! \brief Write the traces specified by the offsets in the passed offset array.
     *  \param[in] sz The number of traces to process
     *  \param[in] offset An array of trace numbers to write.
     *  \param[in] prm A parameter structure
     *  \param[in] skip When writing, skip the first "skip" entries of prm
     *
     *  \details It is assumed that the parameter writing operation is not an update. Any previous
     *  contents of the trace header will be overwritten.
     */
    virtual void writeParam(csize_t sz, csize_t * offset, const Param * prm, csize_t skip = 0) = 0;
};
}}
#endif
