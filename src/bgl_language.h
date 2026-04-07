//
//  bgl_language.h
//  libud
//
//  Created by kejinlu on 2026-02-02
//

#ifndef bgl_language_h
#define bgl_language_h

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Data Structures
// ============================================================

/**
 * @brief Babylon language properties
 */
typedef struct {
    const char *name;        /**< Language name (e.g., "English", "Japanese") */
    const char *name2;       /**< Secondary language name (optional, e.g., "Traditional Chinese") */
    const char *encoding;    /**< Windows code page (e.g., "CP1252", "CP932") */
    int code;                /**< Code value in BGL file (0x00-0x3d) */
} bgl_language;

// ============================================================
// Function Declarations
// ============================================================

/**
 * @brief Get language information by code
 * @param code Language code (0x00-0x3d)
 * @return Language information pointer on success, NULL on failure
 */
const bgl_language *bgl_language_by_code(int code);

/**
 * @brief Get language information by name
 * @param name Language name
 * @return Language information pointer on success, NULL on failure
 */
const bgl_language *bgl_language_by_name(const char *name);

/**
 * @brief Get total number of supported languages
 * @return Total number of languages
 */
int bgl_language_count(void);

/**
 * @brief Iterate through all languages
 * @param index Index (starting from 0)
 * @return Language information pointer on success, NULL if index out of bounds
 */
const bgl_language *bgl_language_at(int index);

// ============================================================
// Charset Mapping (merged from bgl_charset)
// ============================================================

/**
 * @brief Get encoding name by charset code
 * @param code Charset code (0x41-0x4e)
 * @return Encoding name string (e.g., "CP1252") on success, NULL on failure
 *
 * Charset code mapping:
 * 0x41: CP1252 (Default)
 * 0x42: CP1252 (Latin)
 * 0x43: CP1250 (Eastern European)
 * 0x44: CP1251 (Cyrillic)
 * 0x45: CP932 (Japanese)
 * 0x46: CP950 (Traditional Chinese)
 * 0x47: CP936 (Simplified Chinese)
 * 0x48: CP1257 (Baltic)
 * 0x49: CP1253 (Greek)
 * 0x4A: CP949 (Korean)
 * 0x4B: CP1254 (Turkish)
 * 0x4C: CP1255 (Hebrew)
 * 0x4D: CP1256 (Arabic)
 * 0x4E: CP874 (Thai)
 */
const char *bgl_charset_by_code(int code);

/**
 * @brief Get default charset encoding
 * @return Default encoding string ("CP1252")
 */
const char *bgl_charset_default(void);

#ifdef __cplusplus
}
#endif

#endif /* bgl_language_h */
