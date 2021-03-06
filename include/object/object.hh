/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date July 2016
 *   \brief The Object layer interface
 *   \details The Object layer interface is a base class which specific Obj implementations
 *   work off
*//*******************************************************************************************/
#ifndef PIOLOBJ_INCLUDE_GUARD
#define PIOLOBJ_INCLUDE_GUARD
#include "global.hh"

namespace PIOL { namespace Obj {
/*! \brief The Obj layer interface. Specific Obj implementations
 *  work off this base class.
 */
class Interface
{
    protected :
    Piol piol;                              //!< Pointer to the PIOL object.
    std::string name;                       //!< Store the file name for debugging purposes.
    std::shared_ptr<Data::Interface> data;  //!< Pointer to the Data layer object (polymorphic).

    public :
    /*! \brief The constructor.
     *  \param[in] piol_ This PIOL ptr is not modified but is used to instantiate another shared_ptr.
     *  \param[in] name_ The name of the file associated with the instantiation.
     *  \param[in] data_ Pointer to the Data layer object (polymorphic).
     */
    Interface(const Piol piol_, const std::string name_, std::shared_ptr<Data::Interface> data_) : piol(piol_), name(name_), data(data_)
    {
    }

    /*! \brief A virtual destructor to allow deletion.
     */
    virtual ~Interface(void) { }

    /*! \brief Find out the file size.
     *  \return The file size in bytes.
     */
    virtual size_t getFileSz(void) const;

    /*! \brief Set the file size.
     *  \param[in] sz The size in bytes
     */
    virtual void setFileSz(csize_t sz) const;

    /*! \brief Read the header object.
     *  \param[out] ho An array which the caller guarantees is long enough
     *  to hold the header object.
     */
    virtual void readHO(uchar * ho) const = 0;

    /*! \brief Write the header object.
     *  \param[in] ho An array which the caller guarantees is long enough to hold the header object.
     */
    virtual void writeHO(const uchar * ho) const = 0;

    /*! \brief Read a sequence of DOMDs.
     *  \param[in] offset The starting data-object we are interested in.
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of DOMDs to be read in a row.
     *  \param[out] md An array which the caller guarantees is long enough for
     *  the DO metadata.
     */
    virtual void readDOMD(csize_t offset, csize_t ns, csize_t sz, uchar * md) const = 0;

    /*! \brief Write the data-object metadata.
     *  \param[in] offset The starting data-object we are interested in.
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of DOMDs to be written in a row.
     *  \param[in] md An array which the caller guarantees is long enough for
     *  the data-field.
     */
    virtual void writeDOMD(csize_t offset, csize_t ns, csize_t sz, const uchar * md) const = 0;

    /*! \brief Read a sequence of data-fields.
     *  \param[in] offset The starting data-object we are interested in.
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of data-fields to be read in a row.
     *  \param[out] df An array which the caller guarantees is long enough for
     *  the data-field.
     */
    virtual void readDODF(csize_t offset, csize_t ns, csize_t sz, uchar * df) const = 0;

    /*! \brief Write a sequence of data-fields.
     *  \param[in] offset The starting data-object we are interested in.
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of data-fields to be written in a row.
     *  \param[in] df An array which the caller guarantees is long enough for
     *  the data-field.
     */
    virtual void writeDODF(csize_t offset, csize_t ns, csize_t sz, const uchar * df) const = 0;

    /*! \brief Read a sequence of data-objects.
     *  \param[in] offset The starting data-object we are interested in.
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of data-objects to be read in a row.
     *  \param[out] d An array which the caller guarantees is long enough for
     *  the data-objects.
     */
    virtual void readDO(csize_t offset, csize_t ns, csize_t sz, uchar * d) const = 0;

    /*! \brief Write a sequence of data-objects.
     *  \param[in] offset The starting data-object we are interested in.
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of data-objects to be written in a row.
     *  \param[in] d An array which the caller guarantees is long enough for
     *  the data-objects.
     */
    virtual void writeDO(csize_t offset, csize_t ns, csize_t sz, const uchar * d) const = 0;

    /*! \brief Read a list of data-objects.
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of data-objects to be read
     *  \param[in] offset An array of the starting data-objects we are interested in
     *  \param[out] d An array which the caller guarantees is long enough for
     *  the data-objects.
     */
    virtual void readDO(csize_t ns, csize_t sz, csize_t * offset, uchar * d) const = 0;

    /*! \brief Write a list of data-objects.
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of data-objects to be written.
     *  \param[in] offset An array of the starting data-object we are interested in.
     *  \param[in] d An array which the caller guarantees is long enough for
     *  the data-objects.
     */
    virtual void writeDO(csize_t ns, csize_t sz, csize_t * offset, const uchar * d) const = 0;

    /*! \brief read a list of data-object metadata blocks.
     *  \param[in] ns the number of elements per data field.
     *  \param[in] sz the number of DOMD objects to be read
     *  \param[in] offset an array of the starting data-objects we are interested in
     *  \param[out] md an array which the caller guarantees is long enough for
     *  the metadata blocks.
     */
    virtual void readDOMD(csize_t ns, csize_t sz, csize_t * offset, uchar * md) const = 0;

    /*! \brief Write a list of data-object metadata blocks.
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of DOMD objects to be written.
     *  \param[in] offset An array of the starting data-object we are interested in.
     *  \param[in] md An array which the caller guarantees is long enough for
     *  the metadata blocks.
     */
    virtual void writeDOMD(csize_t ns, csize_t sz, csize_t * offset, const uchar * md) const = 0;

    /*! \brief Read a list of data-fields.
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of data-fields to be read
     *  \param[in] offset An array of the starting data-objects we are interested in
     *  \param[out] df An array which the caller guarantees is long enough for
     *  the data-fields.
     */
    virtual void readDODF(csize_t ns, csize_t sz, csize_t * offset, uchar * df) const = 0;

    /*! \brief Write a list of data-fields
     *  \param[in] ns The number of elements per data field.
     *  \param[in] sz The number of data-fields to be written.
     *  \param[in] offset An array of the starting data-object we are interested in.
     *  \param[in] df An array which the caller guarantees is long enough for
     *  the data-fields.
     */
    virtual void writeDODF(csize_t ns, csize_t sz, csize_t * offset, const uchar * df) const = 0;
};
}}
#endif
