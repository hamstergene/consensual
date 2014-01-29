#include "alloc.h"

void *
test_rt_alloc(const void * allocContext, cns_Index size, cns_Error* err)
{
    if (size <= 0)
    {
        *err = CNS_ERR_BADARG;
        return 0;
    }

    cns_Index* rv = (cns_Index*) malloc( sizeof(cns_Index) + size );
    if (rv)
    {
        *rv = size;
        ((struct TestRTAllocContext *)allocContext)->bytesAllocated += size;
    }
    *err = (rv ? CNS_OK : CNS_ERR_NOMEM);
    return rv + 1;
}

void
test_rt_free(const void * allocContext, void* ptr, cns_Error* err)
{
    if (ptr)
    {
        cns_Index* realptr = (cns_Index*) ptr - 1;
        cns_Index size = *realptr;
        ((struct TestRTAllocContext *)allocContext)->bytesAllocated -= size;
        free(realptr);
    }
    *err = CNS_OK;
}

void *
test_rt_realloc(const void * allocContext, void* ptr, cns_Index size, cns_Error* err)
{
    if (size <= 0)
    {
        *err = CNS_ERR_BADARG;
        return 0;
    }

    if (!ptr)
        return test_rt_alloc(allocContext, size, err);

    cns_Index* realptr = (cns_Index*) ptr - 1;
    cns_Index* rv = realloc(realptr, sizeof(cns_Index) + size);
    if (rv)
    {
        ((struct TestRTAllocContext *)allocContext)->bytesAllocated -= *realptr;
        ((struct TestRTAllocContext *)allocContext)->bytesAllocated += size;
        *(cns_Index*)rv = size;
    }
    *err = (rv ? CNS_OK : CNS_ERR_NOMEM);
    return rv + 1;
}

