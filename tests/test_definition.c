//
//  test_definition.c
//  libbgl tests
//
//  Tests for bgl_definition.h definition parsing functions
//

#include "unity.h"
#include "bgl_definition.h"
#include "bgl_port.h"
#include <stdlib.h>
#include <string.h>

// ============================================================
// bgl_parse_definition
// ============================================================

void test_parse_definition_simple(void) {
    // Simple ASCII definition body
    const uint8_t data[] = "Hello world";
    bgl_definition def = {0};
    int result = bgl_parse_definition(data, strlen((const char *)data),
                                       "CP1252", "UTF-8", "CP1252", &def);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_NULL(def.body);
    TEST_ASSERT_EQUAL_STRING("Hello world", def.body);
    bgl_free_definition(&def);
}

void test_parse_definition_empty(void) {
    // NULL data or zero size returns -1
    bgl_definition def = {0};
    int result = bgl_parse_definition(NULL, 0, "CP1252", "UTF-8", "CP1252", &def);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_parse_definition_with_pos(void) {
    // Definition with separator 0x14 then POS field 0x02 0x30 (noun)
    // POS uses abbr first: "n." for noun
    uint8_t data[] = "Hello\x14\x02\x30";
    bgl_definition def = {0};
    int result = bgl_parse_definition(data, sizeof(data) - 1,
                                       "CP1252", "UTF-8", "CP1252", &def);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("n.", def.part_of_speech);
    TEST_ASSERT_NOT_NULL(def.body);
    bgl_free_definition(&def);
}

void test_parse_definition_with_pos_verb(void) {
    // Definition with separator 0x14 then POS field 0x02 0x32 (verb)
    uint8_t data[] = "run\x14\x02\x32";
    bgl_definition def = {0};
    int result = bgl_parse_definition(data, sizeof(data) - 1,
                                       "CP1252", "UTF-8", "CP1252", &def);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("v.", def.part_of_speech);
    bgl_free_definition(&def);
}

void test_parse_definition_with_pos_adj(void) {
    // Definition with separator 0x14 then POS field 0x02 0x31 (adjective)
    uint8_t data[] = "big\x14\x02\x31";
    bgl_definition def = {0};
    int result = bgl_parse_definition(data, sizeof(data) - 1,
                                       "CP1252", "UTF-8", "CP1252", &def);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("adj.", def.part_of_speech);
    bgl_free_definition(&def);
}

// ============================================================
// bgl_free_definition
// ============================================================

void test_free_definition_null_fields(void) {
    bgl_definition def = {0};
    // Should not crash on all-NULL fields
    bgl_free_definition(&def);
    TEST_ASSERT_NULL(def.body);
    TEST_ASSERT_NULL(def.title);
    TEST_ASSERT_NULL(def.title_trans);
    TEST_ASSERT_NULL(def.transcription);
    TEST_ASSERT_NULL(def.field_1a);
}

void test_free_definition_after_parse(void) {
    const uint8_t data[] = "test definition";
    bgl_definition def = {0};
    bgl_parse_definition(data, strlen((const char *)data),
                          "CP1252", "UTF-8", "CP1252", &def);
    bgl_free_definition(&def);
    TEST_ASSERT_NULL(def.body);
}

// ============================================================
// bgl_format_definition
// ============================================================

void test_format_definition_body_only(void) {
    bgl_definition def = {0};
    def.body = bgl_strdup("Definition text");
    char *result = bgl_format_definition(&def);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("Definition text", result);
    free(result);
    free(def.body);
}

void test_format_definition_with_pos(void) {
    bgl_definition def = {0};
    def.part_of_speech = "noun";
    def.title = bgl_strdup("cat");
    def.body = bgl_strdup("a small domesticated feline");
    char *result = bgl_format_definition(&def);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_NOT_NULL(strstr(result, "noun"));
    TEST_ASSERT_NOT_NULL(strstr(result, "cat"));
    free(result);
    free(def.title);
    free(def.body);
}

void test_format_definition_with_transcription(void) {
    bgl_definition def = {0};
    def.body = bgl_strdup("hello");
    def.transcription = bgl_strdup("heloʊ");
    char *result = bgl_format_definition(&def);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_NOT_NULL(strstr(result, "heloʊ"));
    free(result);
    free(def.body);
    free(def.transcription);
}

void test_format_definition_all_null(void) {
    bgl_definition def = {0};
    char *result = bgl_format_definition(&def);
    TEST_ASSERT_NOT_NULL(result);
    free(result);
}

void test_format_definition_title_trans(void) {
    bgl_definition def = {0};
    def.title_trans = bgl_strdup("nya");
    def.body = bgl_strdup("definition");
    char *result = bgl_format_definition(&def);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_NOT_NULL(strstr(result, "nya"));
    free(result);
    free(def.title_trans);
    free(def.body);
}

// ============================================================
// Test Runner
// ============================================================

void run_definition_tests(void) {
    UnityBegin("test_definition.c");

    RUN_TEST(test_parse_definition_simple);
    RUN_TEST(test_parse_definition_empty);
    RUN_TEST(test_parse_definition_with_pos);
    RUN_TEST(test_parse_definition_with_pos_verb);
    RUN_TEST(test_parse_definition_with_pos_adj);

    RUN_TEST(test_free_definition_null_fields);
    RUN_TEST(test_free_definition_after_parse);

    RUN_TEST(test_format_definition_body_only);
    RUN_TEST(test_format_definition_with_pos);
    RUN_TEST(test_format_definition_with_transcription);
    RUN_TEST(test_format_definition_all_null);
    RUN_TEST(test_format_definition_title_trans);
}
