/*******************************************************************************************//*!
 *   \file
 *   \author Cathal O Broin - cathal@ichec.ie - first commit
 *   \copyright TBD. Do not distribute
 *   \date November 2016
 *   \brief The Set layer interface
*//*******************************************************************************************/
#ifndef PIOLSET_INCLUDE_GUARD
#define PIOLSET_INCLUDE_GUARD
#include <functional>
#include <list>
#include <map>
#include "flow/share.hh"
#include "flow/cache.hh"
#include "ops/minmax.hh"
#include "ops/sort.hh"
#include "ops/agc.hh"

//#warning temp
//#include "file/filesegy.hh"
namespace PIOL {
typedef std::function<void(const TraceBlock * in, TraceBlock * out)> Mod;  //!< Typedef for functions that modify traces and associated parameters
typedef std::function<std::vector<size_t>(TraceBlock * data)> InPlaceMod;  //!< Typedef for functions that modify traces and associated parameters
enum class FuncOpt : size_t
{
//Data type dependencies
    NeedMeta,
    NeedTrcVal,
    NeedAll,

//Modification level:
    AddTrc,
    DelTrc,
    ModTrcVal,
    ModTrcLen,
    ModMetaVal,
    ReorderTrc,
    ModAll,

//Modification dependencies
    DepTrcCnt,
    DepTrcOrder,
    DepTrcVal,
    DepMetaVal,
    DepAll,

//Comms level:
    SingleTrace,
    Gather,
    SubSetOnly,
    AllTraces,
    OwnIO
};

struct OpOpt
{
    std::vector<FuncOpt> optList;
    bool check(FuncOpt opt)
    {
        auto it = std::find(optList.begin(), optList.end(), opt);
        return it != optList.end();
    }
    void add(FuncOpt opt)
    {
        optList.push_back(opt);
    }
    OpOpt(void) { }
    OpOpt(std::initializer_list<FuncOpt> list) : optList(list)
    {
    }
};

struct gState
{
    virtual void makeState(const std::vector<size_t> & offset, const Uniray<size_t, llint, llint> & gather) { }
};

struct RadonState : public gState
{
    Piol piol;
    std::vector<trace_t> vtrc;
    std::vector<llint> il;
    std::vector<llint> xl;

    size_t vNs;
    size_t vBin;
    size_t oGSz;
    geom_t vInc;

    RadonState(Piol piol_) : piol(piol_), vNs(0U), vBin(20U), oGSz(60U), vInc(geom_t(0))
    {}

    void makeState(const std::vector<size_t> & offset, const Uniray<size_t, llint, llint> & gather);
};

struct OpParent
{
    OpOpt opt;
    std::shared_ptr<File::Rule> rule;
    std::shared_ptr<gState> state;
    OpParent(OpOpt & opt_, std::shared_ptr<File::Rule> rule_, std::shared_ptr<gState> state_) : opt(opt_), rule(rule_), state(state_) { }
    //OpParent(void) { }
    virtual ~OpParent(void) {}
};

template <typename T>
struct Op : public OpParent
{
    T func;
    Op(OpOpt & opt_, std::shared_ptr<File::Rule> rule_, std::shared_ptr<gState> state_, T func_) : OpParent(opt_, rule_, state_), func(func_)
    {

    }
};

//If this was C++17 then a std::variant could be used
typedef std::list<std::shared_ptr<OpParent>> FuncLst;

/*! The internal set class
 */
class Set
{
    private :
    Piol piol;                                                  //!< The PIOL object.
    std::string outfix;                                         //!< The output prefix
    std::string outmsg;                                         //!< The output text-header message
    FileDeque file;                                             //!< A deque of unique pointers to file descriptors
    std::map<std::pair<size_t, geom_t>, FileDeque> fmap;        //!< A map of (ns, inc) key to a deque of file descriptor pointers
    std::map<std::pair<size_t, geom_t>, size_t> offmap;         //!< A map of (ns, inc) key to the current offset
    std::shared_ptr<File::Rule> rule;                           //!< Contains a pointer to the Rules for parameters
    Cache cache;                                                //!< The cache of parameters and traces
    FuncLst func;                                               //!< The list of functions and related data
    size_t rank;                                                //!< The rank of the particular process
    size_t numRank;                                             //!< The number of ranks

    /*! Fill the file descriptors using the given pattern
     *  \param[in] piol The PIOL object.
     *  \param[in] pattern The file-matching pattern
     */
    void fillDesc(std::shared_ptr<ExSeisPIOL> piol, std::string pattern);

    public :

    /*! Constructor
     *  \param[in] piol_ The PIOL object.
     *  \param[in] pattern The file-matching pattern
     *  \param[in] outfix_ The output file-name prefix
     *  \param[in] rule_ Contains a pointer to the rules to use for trace parameters.
     */
    Set(Piol piol_, std::string pattern, std::string outfix_,
        std::shared_ptr<File::Rule> rule_ = std::make_shared<File::Rule>(std::initializer_list<Meta>{Meta::Copy}));

    /*! Constructor
     *  \param[in] piol_ The PIOL object.
     *  \param[in] pattern The file-matching pattern
     *  \param[in] rule_ Contains a pointer to the rules to use for trace parameters.
     */
    Set(Piol piol_, std::string pattern,
        std::shared_ptr<File::Rule> rule_ = std::make_shared<File::Rule>(std::initializer_list<Meta>{Meta::Copy})) :
        Set(piol_, pattern, "", rule_)
    {}

    /*! Constructor overload
     *  \param[in] piol_ The PIOL object.
     *  \param[in] rule_ Contains a pointer to the rules to use for trace parameters.
     */
    Set(Piol piol_, std::shared_ptr<File::Rule> rule_ = std::make_shared<File::Rule>(std::initializer_list<Meta>{Meta::Copy}))
        : piol(piol_), rule(rule_), cache(piol_)
    {
        rank = piol->comm->getRank();
        numRank = piol->comm->getNumRank();
    }

    /*! Destructor
     */
    ~Set(void);

    /*! Sort the set using the given comparison function
     *  \param[in] sortFunc The comparison function
     */
    void sort(Compare<File::Param> sortFunc);

    /*! Sort the set using the given comparison function
     *  \param[in] r The rules necessary for the sort.
     *  \param[in] sortFunc The comparison function.
     */
    void sort(std::shared_ptr<File::Rule> r, Compare<File::Param> sortFunc);

    /*! Output using the given output prefix
     *  \param[in] oname The output prefix
     *  \return Return a vector of the actual output names.
     */
    std::vector<std::string> output(std::string oname);

    /*! Find the min and max of two given parameters (e.g x and y source coordinates) and return
     *  the associated values and trace numbers in the given structure
     *  \param[in] xlam The function for returning the first parameter
     *  \param[in] ylam The function for returning the second parameter
     *  \param[out] minmax The array of structures to hold the ouput
     */
    void getMinMax(MinMaxFunc<File::Param> xlam, MinMaxFunc<File::Param> ylam, CoordElem * minmax);

    /*! Function to add to modify function that applies a 2 tailed taper to a set of traces
     * \param[in] func Weight function for the taper ramp
     * \param[in] nTailLft Length of left tail of taper
     * \param[in] nTailRt Length of right tail of taper
     */
    void taper(TaperFunc tapFunc, size_t nTailLft, size_t nTailRt = 0);

    /*! Function to add to modify function that applies automatic gain control to a set of traces
     * \param[in] func Staistical function used to scale traces
     * \param[in] window Length of the agc window
     * \param[in] normR Value to which traces are normalized
     */
    void AGC(AGCFunc agcFunc, size_t window, trace_t normR);

    /*! Set the text-header of the output
     *  \param[in] outmsg_ The output message
     */
    void text(std::string outmsg_)
    {
        outmsg = outmsg_;
    }

    /*! Summarise the current status by whatever means the PIOL intrinsically supports
     */
    void summary(void) const;

    /*! Add a file to the set based on the File::ReadInterface
     *  \param[in] in The file interface
     */
    void add(std::unique_ptr<File::ReadInterface> in);

    void drop(void)
    {
        file.resize(0);
        fmap.clear();
        offmap.clear();
    }

    /*! Add a file to the set based on the pattern/name given
     *  \param[in] name The input name
     */
    void add(std::string name);

    /*! Start unwinding the function list for subset-only operations based on the given iterators.
     *  \param[in] fCurr The iterator for the current function to process.
     *  \param[in] fEnd The iterator which indicates the end of the list has been reached.
     *  \return Return the final iterator reached by the last deque.
     */

    FuncLst::iterator startSubset(FuncLst::iterator fCurr, const FuncLst::iterator fEnd);

    /*! Start unwinding the function list for gather operations on the given iterators.
     *  \param[in] fCurr The iterator for the current function to process.
     *  \param[in] fEnd The iterator which indicates the end of the list has been reached.
     *  \return Return a file name if fEnd is reached. Otherwise return "".
     */
    std::string startGather(FuncLst::iterator fCurr, const FuncLst::iterator fEnd);

    /*! Start unwinding the function list for single-trace operations on the given iterators.
     *  \param[in] fCurr The iterator for the current function to process.
     *  \param[in] fEnd The iterator which indicates the end of the list has been reached.
     *  \return Return a list of all file names. Files are currently always created.
     */
    std::vector<std::string> startSingle(FuncLst::iterator fCurr, const FuncLst::iterator fEnd);

    /*! The entry point for unwinding the function list for all use-cases.
     *  \param[in] fCurr The iterator for the current function to process.
     *  \param[in] fEnd The iterator which indicates the end of the list has been reached.
     *  \return Return a list of all file names.
     */
    std::vector<std::string> calcFunc(FuncLst::iterator fCurr, const FuncLst::iterator fEnd);

    /*! The entry point for unwinding the function list for single-traces and gathers only.
     *  \param[in] fCurr The iterator for the current function to process.
     *  \param[in] fEnd The iterator which indicates the end of the list has been reached.
     *  \param[in] type The type of function currently being processed. Either single trace or gather.
     *  \return Return a traceblock which contains the output from the operation.
     *
     *  \description Transitions from gather to single trace are allowed but not the inverse.
     */
    std::unique_ptr<TraceBlock> calcFunc(FuncLst::iterator fCurr, const FuncLst::iterator fEnd, FuncOpt type, const std::unique_ptr<TraceBlock> bIn);

    /*! The entry point for unwinding the function list for subsets.
     *  \param[in] fCurr The iterator for the current function to process.
     *  \param[in] fEnd The iterator which indicates the end of the list has been reached.
     *  \param[in] fQue A deque of unique pointers to file descriptors.
     *  \return Return the final iterator reached.
     */
    FuncLst::iterator calcFuncS(FuncLst::iterator fCurr, const FuncLst::iterator fEnd, FileDeque & fQue);

    /*! Perform the radon to angle conversion. This assumes the input set is a radon transform file.
     *  \param[in] vmName The name of the velocity model file.
     */
    void toAngle(std::string vmName);

    /************************************* Non-Core *****************************************************/
    /*! Sort the set by the specified sort type.
     *  \param[in] type The sort type
     */
    void sort(SortType type);

    /*! Get the min and the max of a set of parameters passed. This is a parallel operation. It is
     *  the collective min and max across all processes (which also must all call this file).
     *  \param[in] m1 The first parameter type
     *  \param[in] m2 The second parameter type
     *  \param[out] minmax An array of structures containing the minimum item.x,  maximum item.x, minimum item.y, maximum item.y
     *  and their respective trace numbers.
     */
    void getMinMax(Meta m1, Meta m2, CoordElem * minmax);

    /*! Perform tailed taper on a set of traces
     * \param[in] type The type of taper to be applied to traces.
     * \param[in] nTailLft The length of left-tail taper ramp.
     * \param[in] nTailRt The length of right-tail taper ramp.
     */
    void taper(TaperType type, size_t nTailLft, size_t nTailRt = 0U);

    /*! Scale traces using automatic gain control for visualization
     * \param[in] type They type of agc scaling function used
     * \param[in] window Length of the agc window
     * \param[in] normR Normalization value
     */
    void AGC(AGCType type, size_t window, trace_t normR);
};
}
#endif