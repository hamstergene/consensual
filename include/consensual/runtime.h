#pragma once

#include <stdint.h>


/**
 * @see cns_lasterr
 */
typedef uint8_t cns_Error;

#define CNS_OK 0
#define CNS_ERR_BADARG 1
#define CNS_ERR_NOMEM 2


/**
 * Used for array indexes, index differences, collection sizes, etc.
 */
typedef intptr_t cns_Index;


typedef struct _cns_Runtime cns_Runtime;


/** Allocates memory.
 * @param allocContext  Value from `cns_runtime_create` call.
 * @param size          Number of bytes to allocate; must be positive.
 * @param out_err       You write allocation result code here; required.
 * @see cns_runtime_create
 */
typedef void * (*cns_Runtime_AllocFn)(const void * allocContext, cns_Index size, cns_Error* out_err);

/** Frees memory.
 * @param allocContext  Value from `cns_runtime_create` call.
 * @param ptr           Pointer to memory allocated by `cns_Runtime_AllocFn` call. May be null.
 * @param out_err       You write allocation result code here; required.
 */
typedef void (*cns_Runtime_FreeFn)(const void * allocContext, void* ptr, cns_Error* out_err);

/** Reallocates memory.
 * If `ptr` is null, works similar to `cns_Runtime_AllocFn`. If this function fails, the previous pointer is not freed.
 * @param allocContext  Value from `cns_runtime_create` call.
 * @param ptr           Pointer to memory allocated by `cns_Runtime_AllocFn` call. May be null.
 * @param size          Number of bytes to allocate; must be positive.
 * @param out_err       You write allocation result code here; required.
 */
typedef void * (*cns_Runtime_ReallocFn)(const void * allocContext, void* ptr, cns_Index size, cns_Error* out_err);

/** Create and initialize Consensual library.
 *
 * All API calls require the same `cns_Runtime*` object, which manages memory allocation, error handling, etc.
 *
 * @param allocfn       Alloc function, like `malloc()`.
 * @param freefn        Free function, like `free()`.
 * @param reallocfn     Realloc function, like `realloc()`.
 * @param allocContext  User-supplied arbitrary pointer value, will be passed to allocation functions.
 * @see cns_shutdown
 */
cns_Runtime *
cns_startup(cns_Runtime_AllocFn allocfn, cns_Runtime_FreeFn freefn, cns_Runtime_ReallocFn reallocfn, const void * allocContext);

/** Shuts down Consensual library.
 *
 * No APIs can be called after this call. If there are objects created under this runtime and still living, they will leak.
 */
void
cns_shutdown(cns_Runtime* cns);


/**
 */
void *
cns_runtime_alloc(cns_Runtime*, cns_Index size);

/**
 * If `ptr` is null, succeeds without doing anything.
 */
void
cns_runtime_free(cns_Runtime*, void* ptr);

/**
 */
void *
cns_runtime_realloc(cns_Runtime*, void* ptr, cns_Index size);


/**
 */
cns_Error
cns_lasterr(cns_Runtime* cns);

/**
 */
void
cns_setlasterr(cns_Runtime* cns, cns_Error errcode);

