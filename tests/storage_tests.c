#include <consensual/runtime.h>
#include <consensual/storage.h>
#include <consensual/bytes.h>
#include "alloc.h"

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static
cns_Bytes* bytesStrFromInt(cns_Runtime* cns, int x)
{
    char buf[40];
    sprintf(buf, "%x", x);
    return cns_bytes_new(cns, buf, strlen(buf) + 1); // never forget the null terminator
}

static
int intFromBytesStr(cns_Runtime* cns, cns_Bytes* value)
{
    int rv = 0;
    sscanf((const char *) cns_bytes_ptr(cns, value), "%x", &rv);
    return rv;
}

START_TEST(test_defaultBytesHash32)
{
    struct TestRTAllocContext test_rt_allocContext = {
        .bytesAllocated = 0,
    };
    cns_Runtime* cns = cns_startup(test_rt_alloc, test_rt_free, test_rt_realloc, &test_rt_allocContext);

    const int noleaksNumber = test_rt_allocContext.bytesAllocated;

    // Measure hash quality using formula from Red Dragon Book:
    // sum( (b[j]*(b[j]+1)/2) / ( (n / (2*m))*(n + 2*m - 1) ) for j in range(m) )
    // b[j] : number of items placed into j-th bucket
    // m    : number of buckets
    // n    : number of items
    // Ideal hash will yield 1.0
    // Most good hash functions yield [0.95..1.05]

    int m = 10000;
    int n = 500000;
    int* b = (int*) cns_runtime_alloc(cns, m * sizeof(int));
    memset(b, 0, m * sizeof(int));

    for (int i = 0; i < n; ++i)
    {
        cns_Bytes* intstr = bytesStrFromInt(cns, i);
        uint32_t intstrhash = cns_storage_defaultBytesHash32(cns, intstr);
        b[intstrhash % m] += 1;
        cns_bytes_free(cns, intstr);
    }
    double noms = 0.0;
    for (int j = 0; j < m; ++j)
    {
        noms += b[j] * (b[j] + 1);
    }
    double q = noms * m / n / (n + 2*m - 1);
    ck_assert_int_le( (int)(q * 100), 105);

    cns_runtime_free(cns, b);

    ck_assert_int_eq( noleaksNumber, test_rt_allocContext.bytesAllocated );

    cns_shutdown(cns);
}
END_TEST

START_TEST(test_storage)
{
    struct TestRTAllocContext test_rt_allocContext = {
        .bytesAllocated = 0,
    };
    cns_Runtime* cns = cns_startup(test_rt_alloc, test_rt_free, test_rt_realloc, &test_rt_allocContext);

    const int noleaksNumber = test_rt_allocContext.bytesAllocated;

    cns_Storage* storage = cns_storage_newMemoryStorage(cns, 0);

    // who tests the tests?
    for (int i = -100; i < 100; i += 17)
    {
        cns_Bytes* bytesStr = bytesStrFromInt(cns, i);
        ck_assert_int_eq(i, intFromBytesStr(cns, bytesStr));
        cns_bytes_free(cns, bytesStr);
    }

    // put 10 values, prove they can be read back
    for (int i = 0; i < 10; ++i)
    {
        cns_Bytes* key = bytesStrFromInt(cns, i);
        int val = -i - 7;
        cns_Bytes* value = bytesStrFromInt(cns, val);

        cns_storage_set(cns, storage, key, value);
        ck_assert_int_eq(CNS_OK, cns_lasterr(cns));

        cns_bytes_free(cns, value);
        cns_bytes_free(cns, key);
    }
    ck_assert_int_eq(10, cns_storage_count(cns, storage));
    for (int i = -14; i < 20; ++i)
    {
        cns_Bytes* key = bytesStrFromInt(cns, i);

        cns_Bytes* value = cns_storage_get(cns, storage, key);
        if (i >= 0 && i < 10)
        {
            ck_assert_int_eq(CNS_OK, cns_lasterr(cns));
            ck_assert_ptr_ne(0, value);
            ck_assert_int_eq(-i - 7, intFromBytesStr(cns, value));
            cns_bytes_free(cns, value);
        }
        else
        {
            ck_assert_ptr_eq(0, value);
        }

        cns_bytes_free(cns, key);
    }

    // put 990 more to cause rehashing, prove all 1000 are intact
    for (int i = 10; i < 1000; ++i)
    {
        cns_Bytes* key = bytesStrFromInt(cns, i);
        int val = -i - 5;
        cns_Bytes* value = bytesStrFromInt(cns, val);

        cns_storage_set(cns, storage, key, value);
        ck_assert_int_eq(CNS_OK, cns_lasterr(cns));

        cns_bytes_free(cns, value);
        cns_bytes_free(cns, key);
    }
    ck_assert_int_eq(1000, cns_storage_count(cns, storage));
    for (int i = -10; i < 1000 + 20; ++i)
    {
        cns_Bytes* key = bytesStrFromInt(cns, i);

        cns_Bytes* value = cns_storage_get(cns, storage, key);
        if (i >= 0 && i < 1000)
        {
            ck_assert_int_eq(CNS_OK, cns_lasterr(cns));
            ck_assert_ptr_ne(0, value);
            if (i < 10)
                ck_assert_int_eq(-i - 7, intFromBytesStr(cns, value));
            else
            {
                if (-i -5 != intFromBytesStr(cns, value))
                {
                    printf("*** mismatch at i=%i length=%d str=%.8s\n", i, (int) cns_bytes_length(cns, value), cns_bytes_ptr(cns, value));
                }
                ck_assert_int_eq(-i - 5, intFromBytesStr(cns, value));
            }
            cns_bytes_free(cns, value);
        }
        else
        {
            ck_assert_ptr_eq(0, value);
        }

        cns_bytes_free(cns, key);
    }

    // test replacing values, replace first 10 with -i - 5
    for (int i = 0; i < 10; ++i)
    {
        cns_Bytes* key = bytesStrFromInt(cns, i);
        int val = -i - 5;
        cns_Bytes* value = bytesStrFromInt(cns, val);

        cns_storage_set(cns, storage, key, value);
        ck_assert_int_eq(CNS_OK, cns_lasterr(cns));

        cns_bytes_free(cns, value);
        cns_bytes_free(cns, key);
    }
    ck_assert_int_eq(1000, cns_storage_count(cns, storage));

    // delete odd numbers, prove they are gone and evens are intact
    for (int i = 0; i < 1000; ++i)
    {
        if (i & 1)
        {
            cns_Bytes* key = bytesStrFromInt(cns, i);

            cns_storage_delete(cns, storage, key);
            ck_assert_int_eq(CNS_OK, cns_lasterr(cns));

            cns_bytes_free(cns, key);
        }
    }
    ck_assert_int_eq(500, cns_storage_count(cns, storage));
    for (int i = 0; i < 1000; ++i)
    {
        cns_Bytes* key = bytesStrFromInt(cns, i);

        if (i & 1)
        {
            ck_assert_ptr_eq(0, cns_storage_get(cns, storage, key));
        }
        else
        {
            cns_Bytes* value = cns_storage_get(cns, storage, key);
            ck_assert_int_eq(CNS_OK, cns_lasterr(cns));
            ck_assert_ptr_ne(0, value);
            ck_assert_int_eq(-i - 5, intFromBytesStr(cns, value));
            cns_bytes_free(cns, value);
        }

        cns_bytes_free(cns, key);
    }

    // delete everything that is not multiple of 10 to cause rehashing, prove the rest is intact
    for (int i = 0; i < 1000; ++i)
    {
        if (i % 5)
        {
            cns_Bytes* key = bytesStrFromInt(cns, i);

            cns_storage_delete(cns, storage, key);
            ck_assert_int_eq(CNS_OK, cns_lasterr(cns));

            cns_bytes_free(cns, key);
        }
    }
    ck_assert_int_eq(100, cns_storage_count(cns, storage));
    for (int i = 0; i < 1000; ++i)
    {
        cns_Bytes* key = bytesStrFromInt(cns, i);

        if (i % 10)
        {
            ck_assert_ptr_eq(0, cns_storage_get(cns, storage, key));
        }
        else
        {
            cns_Bytes* value = cns_storage_get(cns, storage, key);
            ck_assert_int_eq(CNS_OK, cns_lasterr(cns));
            ck_assert_ptr_ne(0, value);
            ck_assert_int_eq(-i - 5, intFromBytesStr(cns, value));
            cns_bytes_free(cns, value);
        }

        cns_bytes_free(cns, key);
    }

    cns_storage_free(cns, storage);

    ck_assert_int_eq( noleaksNumber, test_rt_allocContext.bytesAllocated );

    cns_shutdown(cns);
}
END_TEST

Suite* storage_suite(void)
{
    Suite* s = suite_create("storage");

    TCase* tc = tcase_create("storage");
    tcase_add_test(tc, test_defaultBytesHash32);
    tcase_add_test(tc, test_storage);

    suite_add_tcase(s, tc);
    return s;
}

