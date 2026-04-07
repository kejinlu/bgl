//
//  test_main.c
//  libbgl tests
//
//  Unity-based test runner for libbgl
//

#include "unity.h"
#include <stdio.h>

// Test suite declarations
void run_util_tests(void);
void run_pos_tests(void);
void run_language_tests(void);
void run_text_tests(void);
void run_definition_tests(void);
void run_info_tests(void);
void run_reader_tests(void);

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("  libbgl Test Suite (Unity Framework)\n");
    printf("========================================\n\n");

    run_util_tests();
    run_pos_tests();
    run_language_tests();
    run_text_tests();
    run_definition_tests();
    run_info_tests();
    run_reader_tests();

    printf("\n");
    printf("========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");

    return UnityEnd();
}
