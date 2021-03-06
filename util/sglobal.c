#include "sglobal.h"
#include <assert.h>
Extent decompose(const size_t sz, const size_t nrank, const size_t rank)
{
    assert(nrank > rank);
    size_t r = sz%nrank;
    size_t q = sz/nrank;
    size_t start = q * rank + MIN(rank, r);
    Extent extent = {.start = start, .sz = MIN(sz - start, q + (rank < r))};
    return extent;
}

char * copyString(const char * oldstr)
{
    size_t len = strlen(oldstr);
    //Note that small sizes can generate false positives with valgrind
    //due to copy optimisations.
    char * newstr = calloc(len+1, 1);
    assert(newstr);
    strncpy(newstr, oldstr, len);
    return newstr;
}
