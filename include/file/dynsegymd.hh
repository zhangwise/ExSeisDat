/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date October 2016
 *   \brief The state of this file is temporarily SEG-Y specific, when new formats are
 *          investigated this file should be split into a format-agnostic component and a
 *          SEG-Y specific component.
 *   \details The SEG-Y implementation of metadata is dynamic to reflect the actual usage
 *   by geophysicists where SEG-Y metadata fields are routinely used for other purposes.
 *   as such, the SEG-Y standard is treated as a default series of rules which can be removed
 *   and replaced. A rule specifies a specific type of metadata and particular locations
 *   associated with that rule. For example, each Float rule requires two locations to be
 *   specified, a location for a corresponding integer and a location for a scaling factor.
 *//*******************************************************************************************/
#ifndef PIOLFILEDYNSEGYMD_INCLUDE_GUARD
#define PIOLFILEDYNSEGYMD_INCLUDE_GUARD
#include <unordered_map>
#include <initializer_list>
#include "global.hh"
#include "file/file.hh"
#include "share/param.hh"
#include "share/api.hh"
#include "share/segy.hh"
namespace PIOL { namespace File {
/*! SEG-Y Trace Header offsets
 */
enum class Tr : size_t
{
    SeqNum      = 1U,   //!< int32_t. The trace sequence number in the Line.
    SeqFNum     = 5U,   //!< int32_t. The trace sequence number in SEG-Y File.
    ORF         = 9U,   //!< int32_t. The original field record number.
    TORF        = 13U,  //!< int32_t. The trace number in the ORF.
    ENSrcNum    = 17U,  //!< int32_t. The source energy number.
    SeqNumEns   = 25U,  //!< int32_t. The trace number in the ensemble.
    TIC         = 29U,  //!< int16_t. The trace identification number.
    VStackCnt   = 31U,  //!< int16_t. The number of traces vertically stacked.
    HStackCnt   = 33U,  //!< int16_t. The number of traces horizontally stacked.
    CDist       = 37U,  //!< int32_t. The distance from source center to receiver centre.
    RcvElv      = 41U,  //!< int32_t. The receiver group elevation.
    SurfElvSrc  = 45U,  //!< int32_t. The surface elevation at the source.
    SrcDpthSurf = 49U,  //!< int32_t. The source depth below surface (opposite of above?).
    DtmElvRcv   = 53U,  //!< int32_t. The datum elevation for the receiver group.
    DtmElvSrc   = 57U,  //!< int32_t. The datum elevation for the source.
    WtrDepSrc   = 61U,  //!< int32_t. The water depth for the source.
    WtrDepRcv   = 65U,  //!< int32_t. The water depth for the receive group.
    ScaleElev   = 69U,  //!< int16_t. The scale coordinate for 41-68 (elevations + depths).
    ScaleCoord  = 71U,  //!< int16_t. The scale coordinate for 73-88 + 181-188
    xSrc        = 73U,  //!< int32_t. The X coordinate for the source
    ySrc        = 77U,  //!< int32_t. The Y coordinate for the source
    xRcv        = 81U,  //!< int32_t. The X coordinate for the receive group
    yRcv        = 85U,  //!< int32_t. The Y coordinate for the receive group
    UpSrc       = 95U,  //!< int16_t. The uphole time at the source (ms).
    UpRcv       = 97U,  //!< int16_t. The uphole time at the receive group (ms).
    Ns          = 115U, //!< int16_t. The number of samples in the trace.
    Inc         = 117U, //!< int16_t. The sample interval (us).
    xCmp        = 181U, //!< int32_t. The X coordinate for the CMP
    yCmp        = 185U, //!< int32_t. The Y coordinate for the CMP
    il          = 189U, //!< int32_t. The Inline grid point.
    xl          = 193U, //!< int32_t. The Crossline grid point.
    ShotNum     = 197U, //!< int32_t. The source nearest to the CDP.
    ShotScal    = 201U, //!< int16_t. The shot number scalar. (Explicitly says that 0 == 1)
    ValMeas     = 203U, //!< int16_t. The unit system used for trace values.
    TransConst  = 205U, //!< int32_t. The transduction constant.
    TransExp    = 209U, //!< int16_t. The transduction exponent.
    TransUnit   = 211U, //!< int16_t. The transduction units
    TimeScal    = 215U, //!< int16_t. Scalar for time measurements.
    SrcMeas     = 225U, //!< int32_t. Source measurement.
    SrcMeasExp  = 229U, //!< int16_t. Source measurement exponent.
};

#if defined(__INTEL_COMPILER) || __GNUC__ < 6    //Compiler defects
/*! This function exists to address a defect in enum usage in a map
 *  that is present in the intel and older GNU compilers.
 */
struct EnumHash
{
    /*! This overload describes how to convert from the enum to a size_t
     * \tparam T The enum type
     * \param[in] t The enum value
     * \return Return a cast to size_t
     */
    template <typename T>
    size_t operator()(T t) const
    {
        return static_cast<size_t>(t);
    }
};
#endif

/*! The type of data
 */
enum class MdType : size_t
{
    Long,   //!< Long int data
    Short,  //!< Short int data
    Float,  //!< Floating point data
    Index,  //!< For indexing purposes
    Copy    //!< Copy all relevant headers. Not file format agnostic.
};

/*! An instantiation of this structure corresponds to a single metadata rule
 *
 */
struct RuleEntry
{
    size_t num;     //!< A counter showing this is the numth rule for the given data type.
    size_t loc;     //!< The memory location for the primary data.

    /*! The constructor for storing the rule number and location.
     *  \param[in] num_ The numth rule of the given type for indexing
     *  \param[in] loc_ The location of the primary data
     */
    RuleEntry(csize_t num_, csize_t loc_) : num(num_), loc(loc_) { }

    /*! Virtual destructor to allow overriding by derived classes.
     */
    virtual ~RuleEntry() { };

    /*! Pure virtual function to return the minimum location stored, in derived cases, more data can be stored than
     *  just loc
     *  \return the minimum location
     */
    virtual size_t min(void) = 0;

    /*! Pure virtual function to return the maximum location stored up to, in derived cases, more data can be stored than
     *  just loc
     *  \return the maximum location
     */
    virtual size_t max(void) = 0;

    /*! Pure virtual function to return the datatype.
     *  \return The MdType associated with the derived class.
     */
    virtual MdType type(void) = 0;
};

/*! The Long rule entry structure for the SEG-Y format.
 */
struct SEGYLongRuleEntry : public RuleEntry
{
    /*! The constructor.
     *  \param[in] num_ The numth entry for indexing purposes
     *  \param[in] loc_  The location of the primary data
     */
    SEGYLongRuleEntry(size_t num_, Tr loc_) : RuleEntry(num_, size_t(loc_)) { }

    /*! Return the minimum location stored, i.e loc
     *  just loc
     *  \return the minimum location
     */
    size_t min(void)
    {
        return loc;
    }

    /*! Return the maximum location stored up to, including the size of the data stored
     *  \return the maximum location plus 4 bytes to store an int32_t
     */
    size_t max(void)
    {
        return loc+4U;
    }

    /*! Return the datatype associated with the entry.
     *  \return \c MdType::Long
     */
    MdType type(void)
    {
        return MdType::Long;
    }
};

/*! The Copy rule entry structure for the SEG-Y format.
 */
struct SEGYCopyRuleEntry : public RuleEntry
{
    /*! The constructor (empty).
     */
    SEGYCopyRuleEntry(void) : RuleEntry(0U, 0U) { }

    /*! Return the minimum location stored, i.e 0
     *  \return 0U
     */
    size_t min(void)
    {
        return 0U;
    }

    /*! Return the size of the trace header
     *  \return the size of the trace header
     */
    size_t max(void)
    {
        return SEGSz::getMDSz();
    }

    /*! Return the datatype associated with the entry.
     *  \return \c MdType::Copy
     */
    MdType type(void)
    {
        return MdType::Copy;
    }
};

/*! The Index rule entry structure. For indexing without modifying what
 *  will be stored.
 */
struct SEGYIndexRuleEntry : public RuleEntry
{
    /*! The constructor.
     *  \param[in] num_ The numth entry for indexing purposes
     */
    SEGYIndexRuleEntry(size_t num_) : RuleEntry(num_, 0U) { }

    /*! Return 0. nothing stored
     *  \return Return 0
     */
    size_t min(void)
    {
        return 0;
    }

    /*! Return 0. nothing stored
     *  \return Return 0
     */
    size_t max(void)
    {
        return 0;
    }

    /*! Return the datatype associated with the entry.
     *  \return \c MdType::Index
     */
    MdType type(void)
    {
        return MdType::Index;
    }
};

/*! The Short rule entry structure for the SEG-Y format.
 */
struct SEGYShortRuleEntry : public RuleEntry
{
    /*! The constructor.
     *  \param[in] num_ The numth entry for indexing purposes
     *  \param[in] loc_  The location of the primary data
     */
    SEGYShortRuleEntry(size_t num_, Tr loc_) : RuleEntry(num_, size_t(loc_)) { }

    /*! Return the minimum location stored, i.e loc
     *  \return the minimum location
     */
    size_t min(void)
    {
        return loc;
    }

    /*! Return the maximum location stored up to, including the size of the data stored
     *  \return the maximum location plus 2 bytes to store an int16_t
     */
    size_t max(void)
    {
        return loc+2U;
    }

    /*! Return the datatype associated with the entry.
     *  \return \c MdType::Short
     */
    MdType type(void)
    {
        return MdType::Short;
    }
};

/*! The Float rule entry structure for the SEG-Y format.
 */
struct SEGYFloatRuleEntry : public RuleEntry
{
    size_t scalLoc;     //!< The location of the scaler field.

    /*! The constructor.
     *  \param[in] num_ The numth entry for indexing purposes
     *  \param[in] loc_  The location of the primary data
     *  \param[in] scalLoc_ The location of the scaler field.
     */
    SEGYFloatRuleEntry(size_t num_, Tr loc_, Tr scalLoc_)
                            : RuleEntry(num_, size_t(loc_)), scalLoc(size_t(scalLoc_)) { }

    /*! Return the minimum location stored
     *  \return the minimum location
     */
    size_t min(void)
    {
        return std::min(scalLoc, loc);
    }

    /*! Return the maximum location stored up to, including the size of the data stored
     *  \return the maximum location. If the scaler is in a location higher than the
     *  the primary data store then the location + 2U is returned, otherwise the primary
     *  location + 4U is returned.
     */
    size_t max(void)
    {
        return std::max(scalLoc+2U, loc+4U);
    }

    /*! Return the datatype associated with the entry.
     *  \return \c MdType::Float
     */
    MdType type(void)
    {
        return MdType::Float;
    }
};

//TODO: When implementing alternative file formats, this Rule structure must be generalised
#if defined(__INTEL_COMPILER) || __GNUC__ < 6    //Compiler defects
typedef std::unordered_map<Meta, RuleEntry *, EnumHash> RuleMap;    //!< Typedef for RuleMap accounting for a compiler defect
#else
typedef std::unordered_map<Meta, RuleEntry *> RuleMap;              //!< Typedef for the map holding the rules
#endif

/*! The structure which holds the rules associated with the trace parameters in a file.
 *  These rules describe how to interpret the metadata and also how to index the parameter structure of arrays.
 */
struct Rule
{
    size_t numLong;         //!< Number of long rules.
    size_t numFloat;        //!< Number of float rules.
    size_t numShort;        //!< Number of short rules.
    size_t numIndex;        //!< Number of index rules.
    size_t numCopy;         //!< Number of copy rules. either 0 or 1.
    size_t start;           //!< The starting byte position in the SEG-Y header.
    size_t end;             //!< The end byte position (+ 1) in the SEG-Y header.
    struct
    {
        uint32_t badextent; //!< Flag marking if the extent calculation is stale.
        uint32_t fullextent;//!< Flag marking if the full header buffer is processed.
    } flag;                 //!< State flags

    /*! The unordered map which stores all current rules.
     *  A map ensures there are no duplicates. */
    RuleMap translate;

    /*! The constructor for creating a Rule structure with
     *  default rules in place or no rules in place.
     *  \param[in] full Whether the extents are set to the default size or calculated dynamically.
     *  \param[in] defaults Whether the default SEG-Y rules should be set.
     *  \param[in] extra Whether maximum amount of rules should be set. Useful when copying files
     *              through the library.
     */
    Rule(bool full, bool defaults, bool extra = false);

    /*! The constructor for supplying a list of Meta entries which
     *  have default locations associated with them.
     *  \param[in] m A list of meta entries with default entries. Entries without defaults will be ignored.
     *  \param[in] full Whether the extents are set to the default size or calculated dynamically.
     */
    Rule(std::initializer_list<Meta> m, bool full = true);

    /*! The constructor for creating a Rule structure with
     *  default rules in place or no rules in place.
     *  \param[in] translate_ An unordered map to initialise the internal translate object with.
     *  \param[in] full Whether the extents are set to the default size or calculated dynamically.
     */
    Rule(RuleMap translate_, bool full = true);

    /*! The destructor. Deallocates the memory associated with the rule entries.
     */
    ~Rule(void);

    /*! Add a rule for longs.
     *  \param[in] m The Meta entry.
     *  \param[in] loc The location in the SEG-Y DOMD (4 bytes).
     */
    void addLong(Meta m, Tr loc);

    /*! Add a rule for floats.
     *  \param[in] m The Meta entry.
     *  \param[in] loc The location in the SEG-Y DOMD for the primary data to be stored (4 bytes).
     *  \param[in] scalLoc The location in the SEG-Y DOMD for the scaler to be stored (2 bytes).
     */
    void addSEGYFloat(Meta m, Tr loc, Tr scalLoc);

    /*! Add a rule for floats.
     *  \param[in] m The Meta entry.
     *  \param[in] loc The location in the SEG-Y DOMD for the primary data to be stored (2 bytes).
     */
    void addShort(Meta m, Tr loc);

    /*! Add a rule for an index.
     *  \param[in] m The Meta entry.
     */
    void addIndex(Meta m);

    /*! Remove a rule based on the meta entry.
     *  \param[in] m The meta entry.
     */
    void rmRule(Meta m);

    /*! Return the size of the buffer space required for the metadata items when converting to SEG-Y.
     *  \return Return the size.
     */
    size_t extent(void);

    /*! Estimate of the total memory used
     *  \return Return estimate in bytes.
     */
    size_t memUsage(void) const;

    /*! How much memory will each set of parameters require?
     *  \return Amount of memory in bytes.
     */
    size_t paramMem() const;

    /*! Get the rule entry associated with a particular meta entry.
     *  \param[in] entry The meta entry.
     *  \return The associated rule entry.
     */
    RuleEntry * getEntry(Meta entry);
};

//Access
/*! Get the value associated with the particular entry.
 *  \tparam T The type of the value
 *  \param[in] i The trace number
 *  \param[in] entry The meta entry to retrieve.
 *  \param[in] prm The parameter structure
 *  \return Return the value associated with the entry
 */
//prmRet getPrm(const size_t i, const Meta entry, const Param * prm);
template <typename T>
T getPrm(size_t i, Meta entry, const Param * prm)
{
    Rule * r = prm->r.get();
    RuleEntry * id = r->getEntry(entry);
    switch (id->type())
    {
        case MdType::Float :
            return T(prm->f[r->numFloat*i + id->num]);
        break;
        case MdType::Long :
            return T(prm->i[r->numLong*i + id->num]);
        break;
        case MdType::Short :
            return T(prm->s[r->numShort*i + id->num]);
        break;
        case MdType::Index :
            return T(prm->t[r->numIndex*i + id->num]);
        break;
        default :
            return T(0);
        break;
    }
}

/*! Set the value associated with the particular entry.
 *  \tparam T The type of the value
 *  \param[in] i The trace number
 *  \param[in] entry The meta entry to retrieve.
 *  \param[in] ret The parameter return structure which is initialised by passing a geom_t, llint or short.
 *  \param[in] prm The parameter structure
 */
template <typename T>
void setPrm(csize_t i, const Meta entry, T ret, Param * prm)
{
    Rule * r = prm->r.get();
    switch (r->translate[entry]->type())
    {
        case MdType::Long :
        prm->i[i * r->numLong + r->getEntry(entry)->num] = ret;
        break;
        case MdType::Short :
        prm->s[i * r->numShort + r->getEntry(entry)->num] = ret;
        break;
        case MdType::Float :
        prm->f[i * r->numFloat + r->getEntry(entry)->num] = ret;
        break;
        case MdType::Index :
        prm->t[i * r->numIndex + r->getEntry(entry)->num] = ret;
        break;
        case MdType::Copy :
        break;
    }
}

/*! Copy params from one parameter structure to another.
 * \param[in] j The trace number of the source.
 * \param[in] src The source parameter structure.
 * \param[in] k The trace number of the destination.
 * \param[out] dst The destination parameter structure.
 */
void cpyPrm(csize_t j, const Param * src, csize_t k, Param * dst);
}}
#endif
