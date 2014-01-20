#include <check.h>

START_TEST(test_sanity)
{
    ck_assert_int_eq( 2, 2 );
    ck_assert_int_ne( 2, 3 );
}
END_TEST

Suite* misc_suite(void)
{
    Suite* s = suite_create("misc");

    TCase* tc = tcase_create("sanity");
    tcase_add_test(tc, test_sanity);

    suite_add_tcase(s, tc);
    return s;
}

int main()
{
    int numFailedTests = 0;
    Suite* s = misc_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    numFailedTests = srunner_ntests_failed(sr);
    srunner_free(sr);
    return numFailedTests;
}

