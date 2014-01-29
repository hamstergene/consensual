#include "runtime.h"

typedef struct cns_Bytes cns_Bytes;

/**
 * @param ptr   Pointer to copy bytes from.
 * @param size  Size of memory behind `ptr` in bytes.
 */
cns_Bytes*
cns_bytes_new(cns_Runtime* cns, const void * ptr, cns_Index size);

/**
 * You own the object returned and must free it. The copy is cheap, referring same memory block using reference counting.
 *
 * NOTE: The returned pointer is not guaranteed to be different; you must free it regardless.
 */
cns_Bytes*
cns_bytes_copy(cns_Runtime* cns, cns_Bytes* another);

/**
 */
cns_Index
cns_bytes_length(cns_Runtime* cns, cns_Bytes* bytes);

/**
 * You must not modify this memory, as it may be shared with other Bytes objects.
 */
const void *
cns_bytes_ptr(cns_Runtime* cns, cns_Bytes* bytes);

/**
 * You must call `cns_bytes_free` for each previous `cns_bytes_new`/`cns_bytes_copy`.
 */
void
cns_bytes_free(cns_Runtime* cns, cns_Bytes* bytes);

