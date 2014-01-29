#include <consensual/runtime.h>
#include <consensual/bytes.h>
#include <check.h>

#include "alloc.h"

START_TEST(test_bytes)
{
    struct TestRTAllocContext test_rt_allocContext = {
        .bytesAllocated = 0,
    };
    cns_Runtime* cns = cns_startup(test_rt_alloc, test_rt_free, test_rt_realloc, &test_rt_allocContext);

    const int noleaksNumber = test_rt_allocContext.bytesAllocated;

    void* ptr = cns_runtime_alloc(cns, 100500);
    *(int*)ptr = 123;

    cns_Bytes* a = cns_bytes_new(cns, ptr, 100500);
    ck_assert_ptr_ne(a, 0);
    ck_assert_int_eq(CNS_OK, cns_lasterr(cns));
    ck_assert_int_eq(100500, cns_bytes_length(cns, a));
    ck_assert_int_eq(123, *(int*)cns_bytes_ptr(cns, a));

    cns_runtime_free(cns, ptr);

    int x = test_rt_allocContext.bytesAllocated;

    // ensure copying is cheap
    cns_Bytes* b = cns_bytes_copy(cns, a);
    cns_Bytes* c = cns_bytes_copy(cns, a);
    cns_Bytes* d = cns_bytes_copy(cns, b);

    ck_assert_int_eq(123, *(int*)cns_bytes_ptr(cns, b));
    ck_assert_int_eq(123, *(int*)cns_bytes_ptr(cns, c));
    ck_assert_int_eq(123, *(int*)cns_bytes_ptr(cns, d));
    ck_assert_int_lt( test_rt_allocContext.bytesAllocated, x + 100500*3 );

    cns_bytes_free(cns, a);
    cns_bytes_free(cns, b);
    cns_bytes_free(cns, c);
    cns_bytes_free(cns, d);

    ck_assert_int_eq( noleaksNumber, test_rt_allocContext.bytesAllocated );

    cns_shutdown(cns);
}
END_TEST

Suite* bytes_suite(void)
{
    Suite* s = suite_create("bytes");

    TCase* tc = tcase_create("bytes");
    tcase_add_test(tc, test_bytes);

    suite_add_tcase(s, tc);
    return s;
}

