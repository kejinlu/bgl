//
//  test_text.c
//  libbgl tests
//
//  Tests for bgl_text.h text processing functions
//

#include "unity.h"
#include "bgl_text.h"
#include <stdlib.h>
#include <string.h>

// ============================================================
// bgl_strip
// ============================================================

void test_strip_leading_spaces(void) {
    char *s = strdup("  hello");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip(&s));
    TEST_ASSERT_EQUAL_STRING("hello", s);
    free(s);
}

void test_strip_trailing_spaces(void) {
    char *s = strdup("hello  ");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip(&s));
    TEST_ASSERT_EQUAL_STRING("hello", s);
    free(s);
}

void test_strip_both(void) {
    char *s = strdup("  hello  ");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip(&s));
    TEST_ASSERT_EQUAL_STRING("hello", s);
    free(s);
}

void test_strip_no_change(void) {
    char *s = strdup("hello");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip(&s));
    TEST_ASSERT_EQUAL_STRING("hello", s);
    free(s);
}

void test_strip_all_whitespace(void) {
    char *s = strdup("   ");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip(&s));
    TEST_ASSERT_EQUAL_STRING("", s);
    free(s);
}

void test_strip_tabs_and_newlines(void) {
    char *s = strdup("\t\nhello\n\r");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip(&s));
    TEST_ASSERT_EQUAL_STRING("hello", s);
    free(s);
}

void test_strip_null(void) {
    char *s = NULL;
    TEST_ASSERT_EQUAL_INT(-1, bgl_strip(&s));
    TEST_ASSERT_NULL(s);
}

// ============================================================
// bgl_remove_control_chars
// ============================================================

void test_remove_control_chars_basic(void) {
    char *s = strdup("hello\x01world");
    TEST_ASSERT_EQUAL_INT(0, bgl_remove_control_chars(&s));
    TEST_ASSERT_EQUAL_STRING("helloworld", s);
    free(s);
}

void test_remove_control_chars_preserves_tab(void) {
    char *s = strdup("a\tb");
    TEST_ASSERT_EQUAL_INT(0, bgl_remove_control_chars(&s));
    TEST_ASSERT_EQUAL_STRING("a\tb", s);
    free(s);
}

void test_remove_control_chars_preserves_newline(void) {
    char *s = strdup("a\nb");
    TEST_ASSERT_EQUAL_INT(0, bgl_remove_control_chars(&s));
    TEST_ASSERT_EQUAL_STRING("a\nb", s);
    free(s);
}

void test_remove_control_chars_preserves_cr(void) {
    char *s = strdup("a\rb");
    TEST_ASSERT_EQUAL_INT(0, bgl_remove_control_chars(&s));
    TEST_ASSERT_EQUAL_STRING("a\rb", s);
    free(s);
}

void test_remove_control_chars_multiple(void) {
    // \x01-\x08 are control chars, \x00 is also a control char
    char *s = strdup("\x01\x02\x03\x04\x05hello\x06\x07\x08");
    TEST_ASSERT_EQUAL_INT(0, bgl_remove_control_chars(&s));
    TEST_ASSERT_EQUAL_STRING("hello", s);
    free(s);
}

void test_remove_control_chars_no_change(void) {
    char *s = strdup("hello world");
    TEST_ASSERT_EQUAL_INT(0, bgl_remove_control_chars(&s));
    TEST_ASSERT_EQUAL_STRING("hello world", s);
    free(s);
}

// ============================================================
// bgl_remove_newlines
// ============================================================

void test_remove_newlines_lf(void) {
    char *s = strdup("hello\nworld");
    TEST_ASSERT_EQUAL_INT(0, bgl_remove_newlines(&s));
    TEST_ASSERT_EQUAL_STRING("hello world", s);
    free(s);
}

void test_remove_newlines_crlf(void) {
    char *s = strdup("hello\r\nworld");
    TEST_ASSERT_EQUAL_INT(0, bgl_remove_newlines(&s));
    TEST_ASSERT_EQUAL_STRING("hello world", s);
    free(s);
}

void test_remove_newlines_multiple(void) {
    char *s = strdup("hello\n\nworld");
    TEST_ASSERT_EQUAL_INT(0, bgl_remove_newlines(&s));
    TEST_ASSERT_EQUAL_STRING("hello world", s);
    free(s);
}

void test_remove_newlines_no_change(void) {
    char *s = strdup("hello world");
    TEST_ASSERT_EQUAL_INT(0, bgl_remove_newlines(&s));
    TEST_ASSERT_EQUAL_STRING("hello world", s);
    free(s);
}

// ============================================================
// bgl_normalize_newlines
// ============================================================

void test_normalize_newlines_crlf(void) {
    char *s = strdup("hello\r\nworld");
    TEST_ASSERT_EQUAL_INT(0, bgl_normalize_newlines(&s));
    TEST_ASSERT_EQUAL_STRING("hello\nworld", s);
    free(s);
}

void test_normalize_newlines_cr(void) {
    char *s = strdup("hello\rworld");
    TEST_ASSERT_EQUAL_INT(0, bgl_normalize_newlines(&s));
    TEST_ASSERT_EQUAL_STRING("hello\nworld", s);
    free(s);
}

void test_normalize_newlines_consecutive(void) {
    char *s = strdup("hello\n\n\nworld");
    TEST_ASSERT_EQUAL_INT(0, bgl_normalize_newlines(&s));
    TEST_ASSERT_EQUAL_STRING("hello\nworld", s);
    free(s);
}

void test_normalize_newlines_mixed(void) {
    char *s = strdup("a\r\n\r\nb");
    TEST_ASSERT_EQUAL_INT(0, bgl_normalize_newlines(&s));
    TEST_ASSERT_EQUAL_STRING("a\nb", s);
    free(s);
}

// ============================================================
// bgl_strip_dollar_indexes
// ============================================================

void test_strip_dollar_single(void) {
    char *s = strdup("make do$4$/make");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip_dollar_indexes(&s));
    TEST_ASSERT_EQUAL_STRING("make do/make", s);
    free(s);
}

void test_strip_dollar_double(void) {
    char *s = strdup("test$$$$");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip_dollar_indexes(&s));
    TEST_ASSERT_EQUAL_STRING("test", s);
    free(s);
}

void test_strip_dollar_no_match(void) {
    char *s = strdup("hello$world");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip_dollar_indexes(&s));
    TEST_ASSERT_EQUAL_STRING("hello$world", s);
    free(s);
}

void test_strip_dollar_no_dollar(void) {
    char *s = strdup("hello world");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip_dollar_indexes(&s));
    TEST_ASSERT_EQUAL_STRING("hello world", s);
    free(s);
}

// ============================================================
// bgl_strip_html_tags
// ============================================================

void test_strip_html_tags_basic(void) {
    char *s = strdup("<b>Hello</b> World");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip_html_tags(&s));
    TEST_ASSERT_EQUAL_STRING(" Hello  World", s);
    free(s);
}

void test_strip_html_tags_no_tags(void) {
    char *s = strdup("Hello World");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip_html_tags(&s));
    TEST_ASSERT_EQUAL_STRING("Hello World", s);
    free(s);
}

void test_strip_html_tags_nested(void) {
    char *s = strdup("<div><p>text</p></div>");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip_html_tags(&s));
    // Each tag is replaced by a space, so nested tags produce double spaces
    TEST_ASSERT_NOT_NULL(strstr(s, "text"));
    free(s);
}

// ============================================================
// bgl_fix_img_links
// ============================================================

void test_fix_img_links_rs_us(void) {
    char *s = strdup("<IMG src='\x1eimage.png\x1f'>");
    TEST_ASSERT_EQUAL_INT(0, bgl_fix_img_links(&s));
    TEST_ASSERT_EQUAL_STRING("<IMG src='image.png'>", s);
    free(s);
}

void test_fix_img_links_no_change(void) {
    char *s = strdup("<IMG src='image.png'>");
    TEST_ASSERT_EQUAL_INT(0, bgl_fix_img_links(&s));
    TEST_ASSERT_EQUAL_STRING("<IMG src='image.png'>", s);
    free(s);
}

// ============================================================
// bgl_strip_slash_alt_key
// ============================================================

void test_strip_slash_at_start(void) {
    char *s = strdup("/word");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip_slash_alt_key(&s));
    TEST_ASSERT_EQUAL_STRING("word", s);
    free(s);
}

void test_strip_slash_after_space(void) {
    char *s = strdup(" /word");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip_slash_alt_key(&s));
    TEST_ASSERT_EQUAL_STRING(" word", s);
    free(s);
}

void test_strip_slash_no_match(void) {
    char *s = strdup("http://example.com");
    TEST_ASSERT_EQUAL_INT(0, bgl_strip_slash_alt_key(&s));
    TEST_ASSERT_EQUAL_STRING("http://example.com", s);
    free(s);
}

// ============================================================
// bgl_decode_html_entities
// ============================================================

void test_decode_html_entities_amp(void) {
    char *s = strdup("Hello &amp; World");
    TEST_ASSERT_EQUAL_INT(0, bgl_decode_html_entities(&s, BGL_HTML_KEEP_TAGS));
    TEST_ASSERT_EQUAL_STRING("Hello & World", s);
    free(s);
}

void test_decode_html_entities_lt_gt(void) {
    char *s = strdup("&lt;tag&gt;");
    TEST_ASSERT_EQUAL_INT(0, bgl_decode_html_entities(&s, BGL_HTML_KEEP_TAGS));
    TEST_ASSERT_EQUAL_STRING("<tag>", s);
    free(s);
}

void test_decode_html_entities_decimal(void) {
    char *s = strdup("&#65;");
    TEST_ASSERT_EQUAL_INT(0, bgl_decode_html_entities(&s, BGL_HTML_KEEP_TAGS));
    TEST_ASSERT_EQUAL_STRING("A", s);
    free(s);
}

void test_decode_html_entities_no_entities(void) {
    char *s = strdup("Hello World");
    TEST_ASSERT_EQUAL_INT(0, bgl_decode_html_entities(&s, BGL_HTML_KEEP_TAGS));
    TEST_ASSERT_EQUAL_STRING("Hello World", s);
    free(s);
}

void test_decode_html_entities_strip_tags(void) {
    char *s = strdup("<b>Bold &amp; text</b>");
    TEST_ASSERT_EQUAL_INT(0, bgl_decode_html_entities(&s, BGL_HTML_STRIP));
    // Tags should be stripped
    TEST_ASSERT_EQUAL_STRING("Bold & text", s);
    free(s);
}

// ============================================================
// bgl_decode_text (UTF-8 passthrough)
// ============================================================

void test_decode_text_utf8(void) {
    const uint8_t data[] = {'H', 'e', 'l', 'l', 'o'};
    char *result = bgl_decode_text(data, 5, "UTF-8");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("Hello", result);
    free(result);
}

void test_decode_text_empty(void) {
    char *result = bgl_decode_text(NULL, 0, "UTF-8");
    // NULL data should return NULL or empty string
    if (result) free(result);
}

// ============================================================
// Test Runner
// ============================================================

void run_text_tests(void) {
    UnityBegin("test_text.c");

    RUN_TEST(test_strip_leading_spaces);
    RUN_TEST(test_strip_trailing_spaces);
    RUN_TEST(test_strip_both);
    RUN_TEST(test_strip_no_change);
    RUN_TEST(test_strip_all_whitespace);
    RUN_TEST(test_strip_tabs_and_newlines);
    RUN_TEST(test_strip_null);

    RUN_TEST(test_remove_control_chars_basic);
    RUN_TEST(test_remove_control_chars_preserves_tab);
    RUN_TEST(test_remove_control_chars_preserves_newline);
    RUN_TEST(test_remove_control_chars_preserves_cr);
    RUN_TEST(test_remove_control_chars_multiple);
    RUN_TEST(test_remove_control_chars_no_change);

    RUN_TEST(test_remove_newlines_lf);
    RUN_TEST(test_remove_newlines_crlf);
    RUN_TEST(test_remove_newlines_multiple);
    RUN_TEST(test_remove_newlines_no_change);

    RUN_TEST(test_normalize_newlines_crlf);
    RUN_TEST(test_normalize_newlines_cr);
    RUN_TEST(test_normalize_newlines_consecutive);
    RUN_TEST(test_normalize_newlines_mixed);

    RUN_TEST(test_strip_dollar_single);
    RUN_TEST(test_strip_dollar_double);
    RUN_TEST(test_strip_dollar_no_match);
    RUN_TEST(test_strip_dollar_no_dollar);

    RUN_TEST(test_strip_html_tags_basic);
    RUN_TEST(test_strip_html_tags_no_tags);
    RUN_TEST(test_strip_html_tags_nested);

    RUN_TEST(test_fix_img_links_rs_us);
    RUN_TEST(test_fix_img_links_no_change);

    RUN_TEST(test_strip_slash_at_start);
    RUN_TEST(test_strip_slash_after_space);
    RUN_TEST(test_strip_slash_no_match);

    RUN_TEST(test_decode_html_entities_amp);
    RUN_TEST(test_decode_html_entities_lt_gt);
    RUN_TEST(test_decode_html_entities_decimal);
    RUN_TEST(test_decode_html_entities_no_entities);
    RUN_TEST(test_decode_html_entities_strip_tags);

    RUN_TEST(test_decode_text_utf8);
    RUN_TEST(test_decode_text_empty);
}
