#include <string.h>
#include <math.h>
#include <stdio.h>

#include "cfileapi.h"
#include "flow.h"

#include "wraptests/wraptests.h"


int main()
{
    init_wraptests();

    //
    // Testing ExSeis
    //
    PIOL_ExSeisHandle piol = PIOL_ExSeis_new(0);

    PIOL_ExSeisHandle piol_tmp_1 = PIOL_ExSeis_new(PIOL_VERBOSITY_NONE);
    PIOL_ExSeisHandle piol_tmp_2 = PIOL_ExSeis_new(PIOL_VERBOSITY_MINIMAL);
    PIOL_ExSeisHandle piol_tmp_3 = PIOL_ExSeis_new(PIOL_VERBOSITY_EXTENDED);
    PIOL_ExSeisHandle piol_tmp_4 = PIOL_ExSeis_new(PIOL_VERBOSITY_VERBOSE);
    PIOL_ExSeisHandle piol_tmp_5 = PIOL_ExSeis_new(PIOL_VERBOSITY_MAX);

    if(PIOL_ExSeis_getRank(piol)    != 0)  fail_wraptests();
    if(PIOL_ExSeis_getNumRank(piol) != 10) fail_wraptests();
    PIOL_ExSeis_barrier(piol);

    if(PIOL_ExSeis_max(piol, 0)  != 30) fail_wraptests();
    if(PIOL_ExSeis_max(piol, 40) != 50) fail_wraptests();

    PIOL_ExSeis_isErr(piol, NULL);
    PIOL_ExSeis_isErr(piol, "Test isErr message");

    PIOL_ExSeis_delete(piol_tmp_5);
    PIOL_ExSeis_delete(piol_tmp_4);
    PIOL_ExSeis_delete(piol_tmp_3);
    PIOL_ExSeis_delete(piol_tmp_2);
    PIOL_ExSeis_delete(piol_tmp_1);

    //
    // Testing Rule
    //

    PIOL_File_RuleHandle rule = PIOL_File_Rule_new(true);

    PIOL_File_RuleHandle rule_tmp = PIOL_File_Rule_new(false);

    const PIOL_Meta meta_list[] = {
        PIOL_META_COPY,
        PIOL_META_ltn,
        PIOL_META_gtn,
        PIOL_META_tnl,
        PIOL_META_tnr,
        PIOL_META_tn,
        PIOL_META_tne,
        PIOL_META_ns,
        PIOL_META_inc,
        PIOL_META_Tic,
        PIOL_META_SrcNum,
        PIOL_META_ShotNum,
        PIOL_META_VStack,
        PIOL_META_HStack,
        PIOL_META_Offset,
        PIOL_META_RGElev,
        PIOL_META_SSElev,
        PIOL_META_SDElev,
        PIOL_META_WtrDepSrc,
        PIOL_META_WtrDepRcv,
        PIOL_META_xSrc,
        PIOL_META_ySrc,
        PIOL_META_xRcv,
        PIOL_META_yRcv,
        PIOL_META_xCmp,
        PIOL_META_yCmp,
        PIOL_META_il,
        PIOL_META_xl,
        PIOL_META_TransUnit,
        PIOL_META_TraceUnit,
        PIOL_META_dsdr,
        PIOL_META_Misc1,
        PIOL_META_Misc2,
        PIOL_META_Misc3,
        PIOL_META_Misc4
    };
    PIOL_File_RuleHandle rule_tmp2 = PIOL_File_Rule_new_from_list(
        sizeof(meta_list)/sizeof(meta_list[0]), meta_list
    );
    
    PIOL_File_Rule_addLong(rule, PIOL_META_COPY, 60);
    PIOL_File_Rule_addShort(rule, PIOL_META_COPY, 70);
    PIOL_File_Rule_addSEGYFloat(rule, PIOL_META_COPY, 80, 90);
    PIOL_File_Rule_rmRule(rule, PIOL_META_COPY);

    PIOL_File_Rule_delete(rule_tmp);


    //
    // Testing ReadDirect
    //
    PIOL_File_ReadDirectHandle read_direct =
        PIOL_File_ReadDirect_new(piol, "test_readdirect_filename.sgy");

    {

        if(strcmp(PIOL_File_ReadDirect_readText(read_direct), "Test ReadDirect readText") != 0)
        {
            fail_wraptests();
        };

        if(PIOL_File_ReadDirect_readNs(read_direct) != 100) fail_wraptests();
        if(PIOL_File_ReadDirect_readNt(read_direct) != 110) fail_wraptests();
        if(fabs(PIOL_File_ReadDirect_readInc(read_direct) - 120.0) > 1e-5) fail_wraptests();
    //void readTrace(csize_t offset, csize_t sz, trace_t * trace, Param * prm = const_cast<Param *>(PARAM_NULL)) const;
    //void readParam(csize_t offset, csize_t sz, Param * prm) const;
    //void readTrace(csize_t sz, csize_t * offset, trace_t * trace, Param * prm = const_cast<Param *>(PARAM_NULL)) const;

    //void readTraceNonMono(csize_t sz, csize_t * offset, trace_t * trace, Param * prm = const_cast<Param *>(PARAM_NULL)) const;
    //void readParam(csize_t sz, csize_t * offset, Param * prm) const;

    //~ReadDirect(void) { }
    }

    return finalize_wraptests();
}
