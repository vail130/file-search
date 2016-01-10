#define PATH_MAX        4096    /* # chars in a path name including nul */

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../src/filesearch.h"

int write_file_to_buffer(const char *file_path, char *buffer) {
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    realloc(buffer, length);
    fread(buffer, 1, length, f);
    fclose(f);
    return 0;
}


START_TEST(test_filesearch_matches_glob_pattern)
{
    char buff1[PATH_MAX + 1];
    char *fixture_path = getcwd(buff1, PATH_MAX + 1);
    strcat(fixture_path, "/fixtures/");
    
    char buff2[PATH_MAX + 1];
    char *output_file_path = getcwd(buff2, PATH_MAX + 1);
    strcat(output_file_path, "/fixtures/output.test");
   
    unlink(output_file_path);

    Config opts = { GLOB_MODE, ALL_TYPE, 0, output_file_path };
    int retval = filesearch(fixture_path, "*.json", opts);
    ck_assert_msg (retval == 0, "filesearch failed");

    char *buff3 = malloc(1);
    retval = write_file_to_buffer(output_file_path, buff3);

    ck_assert_msg (retval == 0, "write_file_to_buffer failed");
    ck_assert_msg (strstr(buff3, "/filesearch/tests/fixtures/a/a1/test.json") != NULL, "correct file path not found in output file");
    ck_assert_msg (strstr(buff3, "/filesearch/tests/fixtures/c/test") == NULL, "incorrect file path found in output file");

    free(buff3);
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
