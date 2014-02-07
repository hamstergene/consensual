#pragma once

#include "runtime.h"
#include "bytes.h"

typedef struct cns_Storage cns_Storage;

/** 32-bit hash function type for Bytes object.
 */
typedef uint32_t (* cns_Storage_BytesHash32Fn)(cns_Runtime* cns, cns_Bytes* bytes);

/** The default 32-bit hash function.
 */
uint32_t
cns_storage_defaultBytesHash32(cns_Runtime* cns, cns_Bytes* bytes);

/** Creates simplest storage which holds everything in memory.
 * @param byteshashfn   Hash function. Pass `NULL` to use the default one.
 */
cns_Storage*
cns_storage_newMemoryStorage(cns_Runtime* cns, cns_Storage_BytesHash32Fn byteshashfn);

/**
 */
void
cns_storage_free(cns_Runtime* cns, cns_Storage* storage);

/** Set value for key.
 * Both key and value are copied. If there was a previous value for this key, it is replaced.
 */
void
cns_storage_set(cns_Runtime* cns, cns_Storage* storage, cns_Bytes* key, cns_Bytes* value);

/** Get value for key.
 * The returned value is a copy, you own it and must free it. If there is no value for this key, NULL is returned, and the error is set to CNS_ERR_NOTFOUND.
 */
cns_Bytes*
cns_storage_get(cns_Runtime* cns, cns_Storage* storage, cns_Bytes* key);

/** Deletes value for key.
 * Returns `CNS_YES` if value existed for this key, `CNS_NO` if it didn't.
 */
cns_Bool
cns_storage_delete(cns_Runtime* cns, cns_Storage* storage, cns_Bytes* key);

/** Number of values currently in storage.
 */
cns_Index
cns_storage_count(cns_Runtime* cns, cns_Storage* storage);

