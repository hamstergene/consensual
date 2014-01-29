#include <consensual/runtime.h>
#include <check.h>

#include "alloc.h"

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

    // make sure test_rt_alloc returns writable memory
    int* ptr0 = (int*) cns_runtime_alloc(cns, sizeof(int));
    *ptr0 = 999;
    cns_runtime_free(cns, ptr0);
    ck_assert_int_eq( test_rt_allocContext.bytesAllocated, x );

    // make sure test_rt_realloc does the same
    ptr0 = (int*) cns_runtime_alloc(cns, 1);
    ptr0 = (int*) cns_runtime_realloc(cns, ptr0, sizeof(int));
    *ptr0 = 888;
    cns_runtime_free(cns, ptr0);
    ck_assert_int_eq( test_rt_allocContext.bytesAllocated, x );

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

