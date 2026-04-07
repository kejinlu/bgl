//
//  test_pos.c
//  libbgl tests
//
//  Tests for bgl_pos.h part-of-speech functions
//

#include "unity.h"
#include "bgl_pos.h"

// ============================================================
// bgl_pos_is_valid
// ============================================================

void test_pos_is_valid_noun(void) {
    TEST_ASSERT_TRUE(bgl_pos_is_valid(BGL_POS_NOUN));
}

void test_pos_is_valid_min(void) {
    TEST_ASSERT_TRUE(bgl_pos_is_valid(0x30));
}

void test_pos_is_valid_max(void) {
    TEST_ASSERT_TRUE(bgl_pos_is_valid(0x47));
}

void test_pos_is_valid_below_range(void) {
    TEST_ASSERT_FALSE(bgl_pos_is_valid(0x2F));
}

void test_pos_is_valid_above_range(void) {
    TEST_ASSERT_FALSE(bgl_pos_is_valid(0x48));
}

void test_pos_is_valid_zero(void) {
    TEST_ASSERT_FALSE(bgl_pos_is_valid(0x00));
}

void test_pos_is_valid_negative(void) {
    TEST_ASSERT_FALSE(bgl_pos_is_valid(-1));
}

// ============================================================
// bgl_pos_name_by_code
// ============================================================

void test_pos_name_noun(void) {
    TEST_ASSERT_EQUAL_STRING("noun", bgl_pos_name_by_code(BGL_POS_NOUN));
}

void test_pos_name_adjective(void) {
    TEST_ASSERT_EQUAL_STRING("adjective", bgl_pos_name_by_code(BGL_POS_ADJECTIVE));
}

void test_pos_name_verb(void) {
    TEST_ASSERT_EQUAL_STRING("verb", bgl_pos_name_by_code(BGL_POS_VERB));
}

void test_pos_name_adverb(void) {
    TEST_ASSERT_EQUAL_STRING("adverb", bgl_pos_name_by_code(BGL_POS_ADVERB));
}

void test_pos_name_interjection(void) {
    TEST_ASSERT_EQUAL_STRING("interjection", bgl_pos_name_by_code(BGL_POS_INTERJECTION));
}

void test_pos_name_pronoun(void) {
    TEST_ASSERT_EQUAL_STRING("pronoun", bgl_pos_name_by_code(BGL_POS_PRONOUN));
}

void test_pos_name_preposition(void) {
    TEST_ASSERT_EQUAL_STRING("preposition", bgl_pos_name_by_code(BGL_POS_PREPOSITION));
}

void test_pos_name_conjunction(void) {
    TEST_ASSERT_EQUAL_STRING("conjunction", bgl_pos_name_by_code(BGL_POS_CONJUNCTION));
}

void test_pos_name_suffix(void) {
    TEST_ASSERT_EQUAL_STRING("suffix", bgl_pos_name_by_code(BGL_POS_SUFFIX));
}

void test_pos_name_prefix(void) {
    TEST_ASSERT_EQUAL_STRING("prefix", bgl_pos_name_by_code(BGL_POS_PREFIX));
}

void test_pos_name_article(void) {
    TEST_ASSERT_EQUAL_STRING("article", bgl_pos_name_by_code(BGL_POS_ARTICLE));
}

void test_pos_name_abbreviation(void) {
    TEST_ASSERT_EQUAL_STRING("abbreviation", bgl_pos_name_by_code(BGL_POS_ABBREVIATION));
}

void test_pos_name_numeral(void) {
    TEST_ASSERT_EQUAL_STRING("numeral", bgl_pos_name_by_code(BGL_POS_NUMERAL));
}

void test_pos_name_participle(void) {
    TEST_ASSERT_EQUAL_STRING("participle", bgl_pos_name_by_code(BGL_POS_PARTICIPLE));
}

void test_pos_name_masc_noun_adj(void) {
    TEST_ASSERT_EQUAL_STRING("masculine noun and adjective", bgl_pos_name_by_code(BGL_POS_MASC_NOUN_ADJ));
}

void test_pos_name_fem_noun_adj(void) {
    TEST_ASSERT_EQUAL_STRING("feminine noun and adjective", bgl_pos_name_by_code(BGL_POS_FEM_NOUN_ADJ));
}

void test_pos_name_masc_fem_noun_adj(void) {
    TEST_ASSERT_EQUAL_STRING("masculine and feminine noun and adjective", bgl_pos_name_by_code(BGL_POS_MASC_FEM_NOUN_ADJ));
}

void test_pos_name_fem_noun(void) {
    TEST_ASSERT_EQUAL_STRING("feminine noun", bgl_pos_name_by_code(BGL_POS_FEM_NOUN));
}

void test_pos_name_masc_noun(void) {
    TEST_ASSERT_EQUAL_STRING("masculine noun", bgl_pos_name_by_code(BGL_POS_MASC_NOUN));
}

void test_pos_name_invalid(void) {
    TEST_ASSERT_NULL(bgl_pos_name_by_code(0x00));
    TEST_ASSERT_NULL(bgl_pos_name_by_code(0x2F));
    TEST_ASSERT_NULL(bgl_pos_name_by_code(0x48));
}

// ============================================================
// bgl_pos_abbr_by_code
// ============================================================

void test_pos_abbr_noun(void) {
    TEST_ASSERT_EQUAL_STRING("n.", bgl_pos_abbr_by_code(BGL_POS_NOUN));
}

void test_pos_abbr_adjective(void) {
    TEST_ASSERT_EQUAL_STRING("adj.", bgl_pos_abbr_by_code(BGL_POS_ADJECTIVE));
}

void test_pos_abbr_verb(void) {
    TEST_ASSERT_EQUAL_STRING("v.", bgl_pos_abbr_by_code(BGL_POS_VERB));
}

void test_pos_abbr_adverb(void) {
    TEST_ASSERT_EQUAL_STRING("adv.", bgl_pos_abbr_by_code(BGL_POS_ADVERB));
}

void test_pos_abbr_interjection(void) {
    TEST_ASSERT_EQUAL_STRING("interj.", bgl_pos_abbr_by_code(BGL_POS_INTERJECTION));
}

void test_pos_abbr_pronoun(void) {
    TEST_ASSERT_EQUAL_STRING("pron.", bgl_pos_abbr_by_code(BGL_POS_PRONOUN));
}

void test_pos_abbr_preposition(void) {
    TEST_ASSERT_EQUAL_STRING("prep.", bgl_pos_abbr_by_code(BGL_POS_PREPOSITION));
}

void test_pos_abbr_conjunction(void) {
    TEST_ASSERT_EQUAL_STRING("conj.", bgl_pos_abbr_by_code(BGL_POS_CONJUNCTION));
}

void test_pos_abbr_suffix(void) {
    TEST_ASSERT_EQUAL_STRING("suff.", bgl_pos_abbr_by_code(BGL_POS_SUFFIX));
}

void test_pos_abbr_prefix(void) {
    TEST_ASSERT_EQUAL_STRING("pref.", bgl_pos_abbr_by_code(BGL_POS_PREFIX));
}

void test_pos_abbr_article(void) {
    TEST_ASSERT_EQUAL_STRING("art.", bgl_pos_abbr_by_code(BGL_POS_ARTICLE));
}

void test_pos_abbr_abbreviation(void) {
    TEST_ASSERT_EQUAL_STRING("abbr.", bgl_pos_abbr_by_code(BGL_POS_ABBREVIATION));
}

void test_pos_abbr_numeral(void) {
    TEST_ASSERT_EQUAL_STRING("num.", bgl_pos_abbr_by_code(BGL_POS_NUMERAL));
}

void test_pos_abbr_participle(void) {
    TEST_ASSERT_EQUAL_STRING("part.", bgl_pos_abbr_by_code(BGL_POS_PARTICIPLE));
}

void test_pos_abbr_empty_for_unknown(void) {
    TEST_ASSERT_EQUAL_STRING("", bgl_pos_abbr_by_code(0x3B)); // unknown
}

void test_pos_abbr_empty_for_gendered(void) {
    TEST_ASSERT_EQUAL_STRING("", bgl_pos_abbr_by_code(BGL_POS_MASC_NOUN_ADJ));
    TEST_ASSERT_EQUAL_STRING("", bgl_pos_abbr_by_code(BGL_POS_FEM_NOUN_ADJ));
}

void test_pos_abbr_invalid(void) {
    TEST_ASSERT_NULL(bgl_pos_abbr_by_code(0x00));
    TEST_ASSERT_NULL(bgl_pos_abbr_by_code(0x48));
}

// ============================================================
// Test Runner
// ============================================================

void run_pos_tests(void) {
    UnityBegin("test_pos.c");

    RUN_TEST(test_pos_is_valid_noun);
    RUN_TEST(test_pos_is_valid_min);
    RUN_TEST(test_pos_is_valid_max);
    RUN_TEST(test_pos_is_valid_below_range);
    RUN_TEST(test_pos_is_valid_above_range);
    RUN_TEST(test_pos_is_valid_zero);
    RUN_TEST(test_pos_is_valid_negative);

    RUN_TEST(test_pos_name_noun);
    RUN_TEST(test_pos_name_adjective);
    RUN_TEST(test_pos_name_verb);
    RUN_TEST(test_pos_name_adverb);
    RUN_TEST(test_pos_name_interjection);
    RUN_TEST(test_pos_name_pronoun);
    RUN_TEST(test_pos_name_preposition);
    RUN_TEST(test_pos_name_conjunction);
    RUN_TEST(test_pos_name_suffix);
    RUN_TEST(test_pos_name_prefix);
    RUN_TEST(test_pos_name_article);
    RUN_TEST(test_pos_name_abbreviation);
    RUN_TEST(test_pos_name_numeral);
    RUN_TEST(test_pos_name_participle);
    RUN_TEST(test_pos_name_masc_noun_adj);
    RUN_TEST(test_pos_name_fem_noun_adj);
    RUN_TEST(test_pos_name_masc_fem_noun_adj);
    RUN_TEST(test_pos_name_fem_noun);
    RUN_TEST(test_pos_name_masc_noun);
    RUN_TEST(test_pos_name_invalid);

    RUN_TEST(test_pos_abbr_noun);
    RUN_TEST(test_pos_abbr_adjective);
    RUN_TEST(test_pos_abbr_verb);
    RUN_TEST(test_pos_abbr_adverb);
    RUN_TEST(test_pos_abbr_interjection);
    RUN_TEST(test_pos_abbr_pronoun);
    RUN_TEST(test_pos_abbr_preposition);
    RUN_TEST(test_pos_abbr_conjunction);
    RUN_TEST(test_pos_abbr_suffix);
    RUN_TEST(test_pos_abbr_prefix);
    RUN_TEST(test_pos_abbr_article);
    RUN_TEST(test_pos_abbr_abbreviation);
    RUN_TEST(test_pos_abbr_numeral);
    RUN_TEST(test_pos_abbr_participle);
    RUN_TEST(test_pos_abbr_empty_for_unknown);
    RUN_TEST(test_pos_abbr_empty_for_gendered);
    RUN_TEST(test_pos_abbr_invalid);
}
