//
//  test_info.c
//  libbgl tests
//
//  Tests for bgl_info.h info/metadata parsing functions
//

#include "unity.h"
#include "bgl_info.h"
#include <stdlib.h>
#include <string.h>

// ============================================================
// Helper: create a minimal Type 3 block for a string field
// ============================================================
// Type 3 block format (bgl_parse_info_field expects):
//   2 bytes: field code (big-endian, e.g., 0x00 0x01 for BGL_INFO_TITLE)
//   N bytes: value data

static size_t make_info_block(uint8_t code, const char *data, uint8_t *out) {
    size_t len = strlen(data);
    out[0] = 0x00;              // High byte of code
    out[1] = code;              // Low byte of code
    memcpy(out + 2, data, len);
    return 2 + len;
}

// ============================================================
// bgl_parse_info_field - Title
// ============================================================

void test_parse_info_field_title(void) {
    uint8_t block[256];
    size_t size = make_info_block(BGL_INFO_TITLE, "Test Dictionary", block);
    bgl_info info = {0};
    int result = bgl_parse_info_field(block, size, &info);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_NULL(info.title);
    TEST_ASSERT_EQUAL_STRING("Test Dictionary", info.title);
    bgl_free_info(&info);
}

// ============================================================
// bgl_parse_info_field - Author
// ============================================================

void test_parse_info_field_author(void) {
    uint8_t block[256];
    size_t size = make_info_block(BGL_INFO_AUTHOR, "John Doe", block);
    bgl_info info = {0};
    int result = bgl_parse_info_field(block, size, &info);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_NULL(info.author);
    TEST_ASSERT_EQUAL_STRING("John Doe", info.author);
    bgl_free_info(&info);
}

// ============================================================
// bgl_parse_info_field - Email
// ============================================================

void test_parse_info_field_email(void) {
    uint8_t block[256];
    size_t size = make_info_block(BGL_INFO_EMAIL, "test@example.com", block);
    bgl_info info = {0};
    int result = bgl_parse_info_field(block, size, &info);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_NULL(info.email);
    TEST_ASSERT_EQUAL_STRING("test@example.com", info.email);
    bgl_free_info(&info);
}

// ============================================================
// bgl_parse_info_field - Copyright
// ============================================================

void test_parse_info_field_copyright(void) {
    uint8_t block[256];
    size_t size = make_info_block(BGL_INFO_COPYRIGHT, "2026 kejinlu", block);
    bgl_info info = {0};
    int result = bgl_parse_info_field(block, size, &info);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_NULL(info.copyright);
    TEST_ASSERT_EQUAL_STRING("2026 kejinlu", info.copyright);
    bgl_free_info(&info);
}

// ============================================================
// bgl_parse_info_field - Description
// ============================================================

void test_parse_info_field_description(void) {
    uint8_t block[256];
    size_t size = make_info_block(BGL_INFO_DESCRIPTION, "A test dictionary", block);
    bgl_info info = {0};
    int result = bgl_parse_info_field(block, size, &info);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_NULL(info.description);
    TEST_ASSERT_EQUAL_STRING("A test dictionary", info.description);
    bgl_free_info(&info);
}

// ============================================================
// bgl_parse_info_field - Empty string
// ============================================================

void test_parse_info_field_empty_string(void) {
    // Empty string (all zeros after code) is skipped by bgl_parse_info_field
    uint8_t block[16];
    size_t size = make_info_block(BGL_INFO_TITLE, "", block);
    bgl_info info = {0};
    int result = bgl_parse_info_field(block, size, &info);
    TEST_ASSERT_EQUAL_INT(0, result);
    // Empty values are skipped (treated as all-zero), so title remains NULL
    TEST_ASSERT_NULL(info.title);
}

// ============================================================
// bgl_free_info
// ============================================================

void test_free_info_null(void) {
    bgl_info info = {0};
    // Should not crash
    bgl_free_info(&info);
    TEST_ASSERT_NULL(info.title);
    TEST_ASSERT_NULL(info.author);
}

void test_free_info_after_parse(void) {
    uint8_t block[256];
    size_t size = make_info_block(BGL_INFO_TITLE, "Title", block);
    bgl_info info = {0};
    bgl_parse_info_field(block, size, &info);
    bgl_free_info(&info);
    TEST_ASSERT_NULL(info.title);
    TEST_ASSERT_NULL(info.author);
}

// ============================================================
// bgl_info_field_code enum values
// ============================================================

void test_info_field_codes(void) {
    TEST_ASSERT_EQUAL_INT(0x01, BGL_INFO_TITLE);
    TEST_ASSERT_EQUAL_INT(0x02, BGL_INFO_AUTHOR);
    TEST_ASSERT_EQUAL_INT(0x03, BGL_INFO_EMAIL);
    TEST_ASSERT_EQUAL_INT(0x04, BGL_INFO_COPYRIGHT);
    TEST_ASSERT_EQUAL_INT(0x07, BGL_INFO_SOURCE_LANG);
    TEST_ASSERT_EQUAL_INT(0x08, BGL_INFO_TARGET_LANG);
    TEST_ASSERT_EQUAL_INT(0x09, BGL_INFO_DESCRIPTION);
    TEST_ASSERT_EQUAL_INT(0x0c, BGL_INFO_NUM_ENTRIES);
    TEST_ASSERT_EQUAL_INT(0x11, BGL_INFO_FLAGS);
}

// ============================================================
// bgl_flags structure
// ============================================================

void test_bgl_flags_size(void) {
    // bgl_flags should be a small struct
    TEST_ASSERT_LESS_OR_EQUAL(16, (int)sizeof(bgl_flags));
}

// ============================================================
// bgl_timestamp structure
// ============================================================

void test_bgl_timestamp_size(void) {
    TEST_ASSERT_LESS_OR_EQUAL(32, (int)sizeof(bgl_timestamp));
}

// ============================================================
// bgl_info structure
// ============================================================

void test_bgl_info_initial_zero(void) {
    bgl_info info = {0};
    TEST_ASSERT_NULL(info.title);
    TEST_ASSERT_NULL(info.author);
    TEST_ASSERT_NULL(info.email);
    TEST_ASSERT_NULL(info.copyright);
    TEST_ASSERT_NULL(info.description);
    TEST_ASSERT_NULL(info.source_lang);
    TEST_ASSERT_NULL(info.target_lang);
    TEST_ASSERT_NULL(info.source_charset);
    TEST_ASSERT_NULL(info.target_charset);
    TEST_ASSERT_FALSE(info.utf8_mode);
    TEST_ASSERT_EQUAL_INT(0, info.entry_count);
}

// ============================================================
// Test Runner
// ============================================================

void run_info_tests(void) {
    UnityBegin("test_info.c");

    RUN_TEST(test_parse_info_field_title);
    RUN_TEST(test_parse_info_field_author);
    RUN_TEST(test_parse_info_field_email);
    RUN_TEST(test_parse_info_field_copyright);
    RUN_TEST(test_parse_info_field_description);
    RUN_TEST(test_parse_info_field_empty_string);

    RUN_TEST(test_free_info_null);
    RUN_TEST(test_free_info_after_parse);

    RUN_TEST(test_info_field_codes);
    RUN_TEST(test_bgl_flags_size);
    RUN_TEST(test_bgl_timestamp_size);
    RUN_TEST(test_bgl_info_initial_zero);
}
