#include <consensual/runtime.h>

struct _cns_Runtime
{
    cns_Runtime_AllocFn     allocfn;
    cns_Runtime_FreeFn      freefn;
    cns_Runtime_ReallocFn   reallocfn;
    const void *            allocContext;
    cns_Error               lastError;
};

cns_Runtime *
cns_startup(cns_Runtime_AllocFn allocfn, cns_Runtime_FreeFn freefn, cns_Runtime_ReallocFn reallocfn, const void * allocContext)
{
    if (!allocfn || !freefn || !reallocfn)
        return 0;

    cns_Error err = CNS_OK;
    cns_Runtime* rv = allocfn(allocContext, sizeof(cns_Runtime), &err);
    if (rv)
    {
        rv->allocfn         = allocfn;
        rv->freefn          = freefn;
        rv->reallocfn       = reallocfn;
        rv->allocContext    = allocContext;
        rv->lastError       = err;
    }
    return rv;
}

void
cns_shutdown(cns_Runtime* cns)
{
    if (cns)
    {
        cns_Error err = CNS_OK;
        cns->freefn(cns->allocContext, cns, &err);
    }
}

void *
cns_runtime_alloc(cns_Runtime* cns, cns_Index size)
{
    if (!cns)
        return 0;
    void * rv = cns->allocfn(cns->allocContext, size, &cns->lastError);
    return rv;
}

void
cns_runtime_free(cns_Runtime* cns, void* ptr)
{
    if (!cns)
        return;

    if (ptr == cns)
    {
        cns->lastError = CNS_ERR_BADARG;
        return;
    }

    cns->freefn(cns->allocContext, ptr, &cns->lastError);
}

void *
cns_runtime_realloc(cns_Runtime* cns, void* ptr, cns_Index size)
{
    if (!cns)
        return 0;

    void * rv = cns->reallocfn(cns->allocContext, ptr, size, &cns->lastError);
    return rv;
}

cns_Error
cns_lasterr(cns_Runtime* cns)
{
    return cns->lastError;
}

void
cns_setlasterr(cns_Runtime* cns, cns_Error errcode)
{
    cns->lastError = errcode;
}

