#include <consensual/runtime.h>
#include <stdlib.h>

struct TestRTAllocContext
{
    int bytesAllocated;
};

void *
test_rt_alloc(const void * allocContext, cns_Index size, cns_Error* err);

void
test_rt_free(const void * allocContext, void* ptr, cns_Error* err);

void *
test_rt_realloc(const void * allocContext, void* ptr, cns_Index size, cns_Error* err);

