#include <consensual/bytes.h>

#include <string.h> // memcpy

typedef struct _cns_BytesImpl
{
    int         referenceCount;
    cns_Index   length;
} _cns_BytesImpl;


cns_Bytes*
cns_bytes_new(cns_Runtime* cns, const void * ptr, cns_Index size)
{
    if (!cns || size < 0)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return 0;
    }

    if (!ptr)
        size = 0;

    _cns_BytesImpl* impl = cns_runtime_alloc(cns, sizeof(_cns_BytesImpl) + size);
    if (impl)
    {
        impl->referenceCount = 1;
        impl->length = size;
        memcpy(impl + 1, ptr, size);
    }
    return (cns_Bytes*) impl;
}


cns_Bytes*
cns_bytes_copy(cns_Runtime* cns, cns_Bytes* another)
{
    if (!cns || !another)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return 0;
    }
    _cns_BytesImpl* impl = (_cns_BytesImpl*) another;
    impl->referenceCount++;
    cns_setlasterr(cns, CNS_OK);
    return another;
}


cns_Index
cns_bytes_length(cns_Runtime* cns, cns_Bytes* bytes)
{
    if (!cns || !bytes)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return 0;
    }
    _cns_BytesImpl* impl = (_cns_BytesImpl*) bytes;
    cns_setlasterr(cns, CNS_OK);
    return impl->length;
}


const void *
cns_bytes_ptr(cns_Runtime* cns, cns_Bytes* bytes)
{
    if (!cns || !bytes)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return 0;
    }
    _cns_BytesImpl* impl = (_cns_BytesImpl*) bytes;
    cns_setlasterr(cns, CNS_OK);
    return impl + 1;
}

cns_Bool
cns_bytes_equal(cns_Runtime* cns, cns_Bytes* lhs, cns_Bytes* rhs)
{
    if (lhs == rhs)
        return CNS_YES;

    if (!lhs ^ !rhs)
        return CNS_NO;

    cns_Index length = cns_bytes_length(cns, lhs);
    if (length != cns_bytes_length(cns, rhs))
        return CNS_NO;

    return (0 == memcmp(cns_bytes_ptr(cns, lhs), cns_bytes_ptr(cns, rhs), length));
}

void
cns_bytes_free(cns_Runtime* cns, cns_Bytes* bytes)
{
    if (!cns || !bytes)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return;
    }

    _cns_BytesImpl* impl = (_cns_BytesImpl*) bytes;

    if (impl->referenceCount <= 0)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return;
    }

    cns_setlasterr(cns, CNS_OK);
    if (!--impl->referenceCount)
        cns_runtime_free(cns, impl);
}


