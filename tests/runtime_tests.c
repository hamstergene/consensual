#include <consensual/runtime.h>
#include <check.h>

#include <stdlib.h>

struct TestRTAllocContext
{
    int bytesAllocated;
};

void *
test_rt_alloc(const void * allocContext, cns_Index size, cns_Error* err)
{
    if (size <= 0)
    {
        *err = CNS_ERR_BADARG;
        return 0;
    }

    void * rv = malloc( sizeof(cns_Index) + size );
    if (rv)
    {
        *(cns_Index*)rv = size;
        ((struct TestRTAllocContext *)allocContext)->bytesAllocated += size;
    }
    *err = (rv ? CNS_OK : CNS_ERR_NOMEM);
    return rv;
}

void
test_rt_free(const void * allocContext, void* ptr, cns_Error* err)
{
    if (ptr)
    {
        cns_Index size = *(cns_Index*)ptr;
        ((struct TestRTAllocContext *)allocContext)->bytesAllocated -= size;
    }
    free(ptr);
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
    void * rv = realloc(ptr, sizeof(cns_Index) + size);
    if (rv)
    {
        ((struct TestRTAllocContext *)allocContext)->bytesAllocated -= *(cns_Index*)ptr;
        ((struct TestRTAllocContext *)allocContext)->bytesAllocated += size;
        *(cns_Index*)rv = size;
    }
    *err = (rv ? CNS_OK : CNS_ERR_NOMEM);
    return rv;
}


START_TEST(test_runtime)
{
    struct TestRTAllocContext test_rt_allocContext = {
        .bytesAllocated = 0,
    };
    cns_Runtime* cns = cns_startup(test_rt_alloc, test_rt_free, test_rt_realloc, &test_rt_allocContext);
    ck_assert_ptr_ne( 0, cns );
    ck_assert_int_eq( CNS_OK, cns_lasterr(cns) );

    int x = test_rt_allocContext.bytesAllocated;

    ck_assert_ptr_eq( 0, cns_runtime_alloc(cns, -1) ); // test fail
    ck_assert_int_eq( CNS_ERR_BADARG, cns_lasterr(cns) );
    ck_assert_int_eq( test_rt_allocContext.bytesAllocated, x ); // ensure it did nothing

    void* ptr1 = cns_runtime_alloc(cns, 17);
    ck_assert_int_eq( CNS_OK, cns_lasterr(cns) );
    ck_assert_int_eq( test_rt_allocContext.bytesAllocated, x + 17 );
    ck_assert_ptr_ne( 0, ptr1 );

    cns_setlasterr(cns, CNS_ERR_NOMEM);
    ck_assert_int_eq( CNS_ERR_NOMEM, cns_lasterr(cns) );

    void* ptr2 = cns_runtime_alloc(cns, 2);
    ck_assert_int_eq( CNS_OK, cns_lasterr(cns) );
    ck_assert_int_eq( test_rt_allocContext.bytesAllocated, x + 17 + 2 );
    ck_assert_ptr_ne( 0, ptr2 );

    ck_assert_ptr_ne( ptr1, ptr2 );

    void* tmp;

    tmp = cns_runtime_realloc(cns, ptr1, 16);
    ck_assert_int_eq( CNS_OK, cns_lasterr(cns) );
    ck_assert_ptr_ne( 0, tmp );
    ck_assert_int_eq( test_rt_allocContext.bytesAllocated, x + 16 + 2 );
    ptr1 = tmp;

    tmp = cns_runtime_realloc(cns, ptr2, 113);
    ck_assert_int_eq( CNS_OK, cns_lasterr(cns) );
    ck_assert_ptr_ne( 0, tmp );
    ck_assert_int_eq( test_rt_allocContext.bytesAllocated, x + 16 + 113 );
    ptr2 = tmp;

    cns_setlasterr(cns, CNS_ERR_NOMEM);
    cns_runtime_free(cns, ptr1);
    ck_assert_int_eq( CNS_OK, cns_lasterr(cns) );
    ck_assert_int_eq( test_rt_allocContext.bytesAllocated, x + 113 );

    cns_setlasterr(cns, CNS_ERR_NOMEM);
    cns_runtime_free(cns, ptr2);
    ck_assert_int_eq( CNS_OK, cns_lasterr(cns) );
    ck_assert_int_eq( test_rt_allocContext.bytesAllocated, x );

    cns_setlasterr(cns, CNS_ERR_NOMEM);
    cns_runtime_free(cns, 0); // freeing null
    ck_assert_int_eq( CNS_OK, cns_lasterr(cns) );
    ck_assert_int_eq( test_rt_allocContext.bytesAllocated, x );

    cns_shutdown(cns);
}
END_TEST

Suite* runtime_suite(void)
{
    Suite* s = suite_create("runtime");

    TCase* tc = tcase_create("runtime");
    tcase_add_test(tc, test_runtime);

    suite_add_tcase(s, tc);
    return s;
}

