//
//  bgl_pos.h
//  libud
//
//  Created by kejinlu on 2026-02-02
//

#ifndef bgl_pos_h
#define bgl_pos_h

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Part-of-Speech Codes
// ============================================================

/**
 * @brief BGL part-of-speech codes
 * @note Valid range: 0x30-0x47
 *
 * Reference: pyglossary bgl_pos.py and GoldenDict bgl_babylon.hh
 */
typedef enum {
    BGL_POS_NOUN                   = 0x30, /**< Noun */
    BGL_POS_ADJECTIVE              = 0x31, /**< Adjective */
    BGL_POS_VERB                   = 0x32, /**< Verb */
    BGL_POS_ADVERB                 = 0x33, /**< Adverb */
    BGL_POS_INTERJECTION           = 0x34, /**< Interjection */
    BGL_POS_PRONOUN                = 0x35, /**< Pronoun */
    BGL_POS_PREPOSITION            = 0x36, /**< Preposition */
    BGL_POS_CONJUNCTION            = 0x37, /**< Conjunction */
    BGL_POS_SUFFIX                 = 0x38, /**< Suffix */
    BGL_POS_PREFIX                 = 0x39, /**< Prefix */
    BGL_POS_ARTICLE                = 0x3A, /**< Article */
    BGL_POS_UNKNOWN_3B             = 0x3B, /**< Unknown (seen in some dictionaries) */
    BGL_POS_ABBREVIATION           = 0x3C, /**< Abbreviation (common in Hebrew dictionaries) */
    BGL_POS_MASC_NOUN_ADJ          = 0x3D, /**< Masculine noun and adjective */
    BGL_POS_FEM_NOUN_ADJ           = 0x3E, /**< Feminine noun and adjective */
    BGL_POS_MASC_FEM_NOUN_ADJ      = 0x3F, /**< Masculine and feminine noun and adjective */
    BGL_POS_FEM_NOUN               = 0x40, /**< Feminine noun */
    BGL_POS_MASC_FEM_NOUN          = 0x41, /**< Masculine and feminine noun */
    BGL_POS_MASC_NOUN              = 0x42, /**< Masculine noun */
    BGL_POS_NUMERAL                = 0x43, /**< Numeral */
    BGL_POS_PARTICIPLE             = 0x44, /**< Participle */
} bgl_pos_code;

// ============================================================
// Function Declarations
// ============================================================

/**
 * @brief Get part-of-speech full name by code
 * @param code Part-of-speech code (0x30-0x47)
 * @return Full name string (static memory, do not free), or NULL on error
 *
 * @note The returned pointer points to static memory and must not be freed
 */
const char *bgl_pos_name_by_code(int code);

/**
 * @brief Get part-of-speech abbreviation by code
 * @param code Part-of-speech code (0x30-0x47)
 * @return Abbreviation string (static memory, do not free), or NULL on error
 *
 * @note The returned pointer points to static memory and must not be freed
 * @note Returns empty string for codes without abbreviation
 */
const char *bgl_pos_abbr_by_code(int code);

/**
 * @brief Check if a code is a valid part-of-speech code
 * @param code Part-of-speech code to validate
 * @return true if valid (0x30-0x47), false otherwise
 */
bool bgl_pos_is_valid(int code);

#ifdef __cplusplus
}
#endif

#endif /* bgl_pos_h */
