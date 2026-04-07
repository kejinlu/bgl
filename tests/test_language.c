//
//  test_language.c
//  libbgl tests
//
//  Tests for bgl_language.h language and charset functions
//

#include "unity.h"
#include "bgl_language.h"

// ============================================================
// bgl_language_by_code
// ============================================================

void test_language_by_code_english(void) {
    const bgl_language *lang = bgl_language_by_code(0x00);
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_STRING("English", lang->name);
    TEST_ASSERT_EQUAL_STRING("CP1252", lang->encoding);
    TEST_ASSERT_EQUAL_INT(0x00, lang->code);
}

void test_language_by_code_japanese(void) {
    const bgl_language *lang = bgl_language_by_code(0x08);
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_STRING("Japanese", lang->name);
    TEST_ASSERT_EQUAL_STRING("CP932", lang->encoding);
}

void test_language_by_code_chinese_traditional(void) {
    const bgl_language *lang = bgl_language_by_code(0x09);
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_STRING("Chinese", lang->name);
    TEST_ASSERT_EQUAL_STRING("Traditional Chinese", lang->name2);
    TEST_ASSERT_EQUAL_STRING("CP950", lang->encoding);
}

void test_language_by_code_chinese_simplified(void) {
    const bgl_language *lang = bgl_language_by_code(0x0a);
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_STRING("Chinese", lang->name);
    TEST_ASSERT_EQUAL_STRING("Simplified Chinese", lang->name2);
    TEST_ASSERT_EQUAL_STRING("CP936", lang->encoding);
}

void test_language_by_code_korean(void) {
    const bgl_language *lang = bgl_language_by_code(0x0c);
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_STRING("Korean", lang->name);
    TEST_ASSERT_EQUAL_STRING("CP949", lang->encoding);
}

void test_language_by_code_russian(void) {
    const bgl_language *lang = bgl_language_by_code(0x07);
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_STRING("Russian", lang->name);
    TEST_ASSERT_EQUAL_STRING("CP1251", lang->encoding);
}

void test_language_by_code_invalid_negative(void) {
    TEST_ASSERT_NULL(bgl_language_by_code(-1));
}

void test_language_by_code_invalid_out_of_range(void) {
    TEST_ASSERT_NULL(bgl_language_by_code(0x3E));
}

// ============================================================
// bgl_language_by_name
// ============================================================

void test_language_by_name_english(void) {
    const bgl_language *lang = bgl_language_by_name("English");
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_INT(0x00, lang->code);
}

void test_language_by_name_french(void) {
    const bgl_language *lang = bgl_language_by_name("French");
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_INT(0x01, lang->code);
}

void test_language_by_name_german(void) {
    const bgl_language *lang = bgl_language_by_name("German");
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_INT(0x06, lang->code);
}

void test_language_by_name_not_found(void) {
    TEST_ASSERT_NULL(bgl_language_by_name("NonexistentLanguage"));
}

void test_language_by_name_null(void) {
    TEST_ASSERT_NULL(bgl_language_by_name(NULL));
}

void test_language_by_name_empty(void) {
    TEST_ASSERT_NULL(bgl_language_by_name(""));
}

// ============================================================
// bgl_language_count & bgl_language_at
// ============================================================

void test_language_count(void) {
    int count = bgl_language_count();
    TEST_ASSERT_EQUAL_INT(62, count); // 0x00-0x3d
}

void test_language_at_first(void) {
    const bgl_language *lang = bgl_language_at(0);
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_STRING("English", lang->name);
}

void test_language_at_last(void) {
    int count = bgl_language_count();
    const bgl_language *lang = bgl_language_at(count - 1);
    TEST_ASSERT_NOT_NULL(lang);
    TEST_ASSERT_EQUAL_STRING("Somali", lang->name);
}

void test_language_at_negative(void) {
    TEST_ASSERT_NULL(bgl_language_at(-1));
}

void test_language_at_out_of_range(void) {
    int count = bgl_language_count();
    TEST_ASSERT_NULL(bgl_language_at(count));
}

void test_language_at_iteration(void) {
    // Verify iteration through all languages produces consistent results
    int count = bgl_language_count();
    for (int i = 0; i < count; i++) {
        const bgl_language *lang = bgl_language_at(i);
        TEST_ASSERT_NOT_NULL(lang);
        TEST_ASSERT_EQUAL_INT(i, lang->code);
    }
}

// ============================================================
// bgl_charset_by_code
// ============================================================

void test_charset_default_code(void) {
    TEST_ASSERT_EQUAL_STRING("CP1252", bgl_charset_by_code(0x41));
}

void test_charset_latin(void) {
    TEST_ASSERT_EQUAL_STRING("CP1252", bgl_charset_by_code(0x42));
}

void test_charset_eastern_european(void) {
    TEST_ASSERT_EQUAL_STRING("CP1250", bgl_charset_by_code(0x43));
}

void test_charset_cyrillic(void) {
    TEST_ASSERT_EQUAL_STRING("CP1251", bgl_charset_by_code(0x44));
}

void test_charset_japanese(void) {
    TEST_ASSERT_EQUAL_STRING("CP932", bgl_charset_by_code(0x45));
}

void test_charset_traditional_chinese(void) {
    TEST_ASSERT_EQUAL_STRING("CP950", bgl_charset_by_code(0x46));
}

void test_charset_simplified_chinese(void) {
    TEST_ASSERT_EQUAL_STRING("CP936", bgl_charset_by_code(0x47));
}

void test_charset_baltic(void) {
    TEST_ASSERT_EQUAL_STRING("CP1257", bgl_charset_by_code(0x48));
}

void test_charset_greek(void) {
    TEST_ASSERT_EQUAL_STRING("CP1253", bgl_charset_by_code(0x49));
}

void test_charset_korean(void) {
    TEST_ASSERT_EQUAL_STRING("CP949", bgl_charset_by_code(0x4a));
}

void test_charset_turkish(void) {
    TEST_ASSERT_EQUAL_STRING("CP1254", bgl_charset_by_code(0x4b));
}

void test_charset_hebrew(void) {
    TEST_ASSERT_EQUAL_STRING("CP1255", bgl_charset_by_code(0x4c));
}

void test_charset_arabic(void) {
    TEST_ASSERT_EQUAL_STRING("CP1256", bgl_charset_by_code(0x4d));
}

void test_charset_thai(void) {
    TEST_ASSERT_EQUAL_STRING("CP874", bgl_charset_by_code(0x4e));
}

void test_charset_invalid_below(void) {
    TEST_ASSERT_NULL(bgl_charset_by_code(0x40));
}

void test_charset_invalid_above(void) {
    TEST_ASSERT_NULL(bgl_charset_by_code(0x4f));
}

// ============================================================
// bgl_charset_default
// ============================================================

void test_charset_default(void) {
    TEST_ASSERT_EQUAL_STRING("CP1252", bgl_charset_default());
}

// ============================================================
// Test Runner
// ============================================================

void run_language_tests(void) {
    UnityBegin("test_language.c");

    RUN_TEST(test_language_by_code_english);
    RUN_TEST(test_language_by_code_japanese);
    RUN_TEST(test_language_by_code_chinese_traditional);
    RUN_TEST(test_language_by_code_chinese_simplified);
    RUN_TEST(test_language_by_code_korean);
    RUN_TEST(test_language_by_code_russian);
    RUN_TEST(test_language_by_code_invalid_negative);
    RUN_TEST(test_language_by_code_invalid_out_of_range);

    RUN_TEST(test_language_by_name_english);
    RUN_TEST(test_language_by_name_french);
    RUN_TEST(test_language_by_name_german);
    RUN_TEST(test_language_by_name_not_found);
    RUN_TEST(test_language_by_name_null);
    RUN_TEST(test_language_by_name_empty);

    RUN_TEST(test_language_count);
    RUN_TEST(test_language_at_first);
    RUN_TEST(test_language_at_last);
    RUN_TEST(test_language_at_negative);
    RUN_TEST(test_language_at_out_of_range);
    RUN_TEST(test_language_at_iteration);

    RUN_TEST(test_charset_default_code);
    RUN_TEST(test_charset_latin);
    RUN_TEST(test_charset_eastern_european);
    RUN_TEST(test_charset_cyrillic);
    RUN_TEST(test_charset_japanese);
    RUN_TEST(test_charset_traditional_chinese);
    RUN_TEST(test_charset_simplified_chinese);
    RUN_TEST(test_charset_baltic);
    RUN_TEST(test_charset_greek);
    RUN_TEST(test_charset_korean);
    RUN_TEST(test_charset_turkish);
    RUN_TEST(test_charset_hebrew);
    RUN_TEST(test_charset_arabic);
    RUN_TEST(test_charset_thai);
    RUN_TEST(test_charset_invalid_below);
    RUN_TEST(test_charset_invalid_above);

    RUN_TEST(test_charset_default);
}
