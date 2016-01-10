#define PATH_MAX        4096    /* # chars in a path name including nul */

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../src/filesearch.h"

START_TEST(test_filesearch_matches_glob_pattern)
{
    char* cwd;
    char buff[PATH_MAX + 1];
    cwd = getcwd(buff, PATH_MAX + 1);
    puts(cwd);
    strcat(cwd, "/fixtures/");
    Config opts = { GLOB_MODE, ALL_TYPE, 0 };
    int retval = filesearch(cwd, "*", opts);
    ck_assert (retval == 0);
}
END_TEST

Suite * filesearch_suite (void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("Filesearch");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_filesearch_matches_glob_pattern);
    suite_add_tcase(s, tc_core);

    return s;
}

int main (void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = filesearch_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
