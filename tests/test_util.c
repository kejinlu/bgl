//
//  test_util.c
//  libbgl tests
//
//  Tests for bgl_util.h inline utility functions
//

#include "unity.h"
#include "bgl_util.h"
#include <string.h>

// ============================================================
// bgl_read_uint16_be
// ============================================================

void test_read_uint16_be_basic(void) {
    uint8_t data[] = {0x12, 0x34};
    TEST_ASSERT_EQUAL_UINT16(0x1234, bgl_read_uint16_be(data));
}

void test_read_uint16_be_zero(void) {
    uint8_t data[] = {0x00, 0x00};
    TEST_ASSERT_EQUAL_UINT16(0x0000, bgl_read_uint16_be(data));
}

void test_read_uint16_be_max(void) {
    uint8_t data[] = {0xFF, 0xFF};
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, bgl_read_uint16_be(data));
}

void test_read_uint16_be_low_byte(void) {
    uint8_t data[] = {0x00, 0x01};
    TEST_ASSERT_EQUAL_UINT16(0x0001, bgl_read_uint16_be(data));
}

void test_read_uint16_be_high_byte(void) {
    uint8_t data[] = {0x80, 0x00};
    TEST_ASSERT_EQUAL_UINT16(0x8000, bgl_read_uint16_be(data));
}

// ============================================================
// bgl_read_uint32_be
// ============================================================

void test_read_uint32_be_basic(void) {
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    TEST_ASSERT_EQUAL_UINT32(0x12345678, bgl_read_uint32_be(data));
}

void test_read_uint32_be_zero(void) {
    uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_EQUAL_UINT32(0x00000000, bgl_read_uint32_be(data));
}

void test_read_uint32_be_max(void) {
    uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF};
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFF, bgl_read_uint32_be(data));
}

void test_read_uint32_be_low_byte(void) {
    uint8_t data[] = {0x00, 0x00, 0x00, 0x01};
    TEST_ASSERT_EQUAL_UINT32(0x00000001, bgl_read_uint32_be(data));
}

void test_read_uint32_be_high_byte(void) {
    uint8_t data[] = {0x80, 0x00, 0x00, 0x00};
    TEST_ASSERT_EQUAL_UINT32(0x80000000, bgl_read_uint32_be(data));
}

void test_read_uint32_be_mixed(void) {
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    TEST_ASSERT_EQUAL_UINT32(0xDEADBEEF, bgl_read_uint32_be(data));
}

// ============================================================
// bgl_codepoint_to_utf8
// ============================================================

void test_codepoint_to_utf8_ascii(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(1, bgl_codepoint_to_utf8(0x41, buf)); // 'A'
    TEST_ASSERT_EQUAL_UINT8('A', buf[0]);
}

void test_codepoint_to_utf8_2byte(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(2, bgl_codepoint_to_utf8(0xE9, buf)); // 'e' with acute
    TEST_ASSERT_EQUAL_UINT8(0xC3, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0xA9, buf[1]);
}

void test_codepoint_to_utf8_3byte(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(3, bgl_codepoint_to_utf8(0x4E2D, buf)); // CJK 'middle'
    TEST_ASSERT_EQUAL_UINT8(0xE4, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0xB8, buf[1]);
    TEST_ASSERT_EQUAL_UINT8(0xAD, buf[2]);
}

void test_codepoint_to_utf8_4byte(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(4, bgl_codepoint_to_utf8(0x1F600, buf)); // emoji
    TEST_ASSERT_EQUAL_UINT8(0xF0, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0x9F, buf[1]);
    TEST_ASSERT_EQUAL_UINT8(0x98, buf[2]);
    TEST_ASSERT_EQUAL_UINT8(0x80, buf[3]);
}

void test_codepoint_to_utf8_null(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(1, bgl_codepoint_to_utf8(0x00, buf));
    TEST_ASSERT_EQUAL_UINT8(0x00, buf[0]);
}

void test_codepoint_to_utf8_boundary_1(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(1, bgl_codepoint_to_utf8(0x7F, buf));
}

void test_codepoint_to_utf8_boundary_2(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(2, bgl_codepoint_to_utf8(0x80, buf));
}

void test_codepoint_to_utf8_boundary_3(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(2, bgl_codepoint_to_utf8(0x7FF, buf));
}

void test_codepoint_to_utf8_boundary_4(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(3, bgl_codepoint_to_utf8(0x800, buf));
}

void test_codepoint_to_utf8_boundary_5(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(3, bgl_codepoint_to_utf8(0xFFFF, buf));
}

void test_codepoint_to_utf8_boundary_6(void) {
    char buf[4];
    TEST_ASSERT_EQUAL_INT(4, bgl_codepoint_to_utf8(0x10000, buf));
}

void test_codepoint_to_utf8_invalid(void) {
    char buf[4];
    // Beyond Unicode range
    TEST_ASSERT_EQUAL_INT(0, bgl_codepoint_to_utf8(0x110000, buf));
}

// ============================================================
// Test Runner
// ============================================================

void run_util_tests(void) {
    UnityBegin("test_util.c");

    RUN_TEST(test_read_uint16_be_basic);
    RUN_TEST(test_read_uint16_be_zero);
    RUN_TEST(test_read_uint16_be_max);
    RUN_TEST(test_read_uint16_be_low_byte);
    RUN_TEST(test_read_uint16_be_high_byte);

    RUN_TEST(test_read_uint32_be_basic);
    RUN_TEST(test_read_uint32_be_zero);
    RUN_TEST(test_read_uint32_be_max);
    RUN_TEST(test_read_uint32_be_low_byte);
    RUN_TEST(test_read_uint32_be_high_byte);
    RUN_TEST(test_read_uint32_be_mixed);

    RUN_TEST(test_codepoint_to_utf8_ascii);
    RUN_TEST(test_codepoint_to_utf8_2byte);
    RUN_TEST(test_codepoint_to_utf8_3byte);
    RUN_TEST(test_codepoint_to_utf8_4byte);
    RUN_TEST(test_codepoint_to_utf8_null);
    RUN_TEST(test_codepoint_to_utf8_boundary_1);
    RUN_TEST(test_codepoint_to_utf8_boundary_2);
    RUN_TEST(test_codepoint_to_utf8_boundary_3);
    RUN_TEST(test_codepoint_to_utf8_boundary_4);
    RUN_TEST(test_codepoint_to_utf8_boundary_5);
    RUN_TEST(test_codepoint_to_utf8_boundary_6);
    RUN_TEST(test_codepoint_to_utf8_invalid);
}
