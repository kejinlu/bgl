//
//  bgl_info.h
//  libud
//
//  Created by kejinlu on 2026-02-02
//
//  NOTE: This module handles parsing of individual Type 3 info blocks from BGL files.
//
//  Naming and Relationship:
//  - "info field" = Single metadata field (e.g., title, author, encoding) from one Type 3 block
//  - "info" = Collection/aggregation of all info fields parsed from the BGL file
//
//  Workflow: Type 3 Block (single field) → bgl_parse_info_field() → bgl_info (collection)
//

#ifndef bgl_info_h
#define bgl_info_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "bgl_language.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Data Structures
// ============================================================

/**
 * @brief Type 3 metadata flags
 */
typedef struct {
    bool utf8_encoding;         /**< Use UTF-8 encoding (flags & 0x8000) */
    bool spelling_alternatives; /**< Spelling alternatives (flags & 0x10000 == 0) */
    bool case_sensitive;        /**< Case sensitive (flags & 0x1000) */
} bgl_flags;

/**
 * @brief Babylon timestamp (BGL binary format)
 */
typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
} bgl_timestamp;

/**
 * @brief Type 3 metadata item types
 */
typedef enum {
    BGL_INFO_TITLE = 0x01,              /**< Dictionary title */
    BGL_INFO_AUTHOR = 0x02,             /**< Author */
    BGL_INFO_EMAIL = 0x03,              /**< Author email */
    BGL_INFO_COPYRIGHT = 0x04,          /**< Copyright information */
    BGL_INFO_SOURCE_LANG = 0x07,        /**< Source language */
    BGL_INFO_TARGET_LANG = 0x08,        /**< Target language */
    BGL_INFO_DESCRIPTION = 0x09,        /**< Description */
    BGL_INFO_BROWSING_ENABLED = 0x0a,   /**< Browsing enabled */
    BGL_INFO_ICON1 = 0x0b,              /**< Icon 1 */
    BGL_INFO_NUM_ENTRIES = 0x0c,        /**< Number of entries */
    BGL_INFO_FLAGS = 0x11,              /**< Flags (UTF-8, etc.) */
    BGL_INFO_CREATION_TIME = 0x14,      /**< Creation time */
    BGL_INFO_SOURCE_CHARSET = 0x1a,     /**< Source charset */
    BGL_INFO_TARGET_CHARSET = 0x1b,     /**< Target charset */
    BGL_INFO_FIRST_UPDATED = 0x1c,      /**< First update time */
    BGL_INFO_CASE_SENSITIVE2 = 0x20,    /**< Case sensitive 2 */
    BGL_INFO_ICON2 = 0x24,              /**< Icon 2 */
    BGL_INFO_PURCHASE_LICENSE_MSG = 0x2c, /**< Purchase license message */
    BGL_INFO_LICENSE_EXPIRED_MSG = 0x2d,  /**< License expired message */
    BGL_INFO_PURCHASE_ADDRESS = 0x2e,   /**< Purchase address */
    BGL_INFO_TITLE_WIDE = 0x30,         /**< Wide title */
    BGL_INFO_AUTHOR_WIDE = 0x31,        /**< Wide author */
    BGL_INFO_LAST_UPDATED = 0x33,       /**< Last update time */
    BGL_INFO_CONTRACTIONS = 0x3b,       /**< Contractions */
    BGL_INFO_FONT_NAME = 0x3d,          /**< Font name */
    BGL_INFO_ABOUT = 0x41,              /**< About information */
    BGL_INFO_LENGTH = 0x43,             /**< Substring match length */
} bgl_info_field_code;

// ============================================================
// Info Collection Structure
// ============================================================

/**
 * @brief BGL dictionary info collection (aggregation of all Type 3 info blocks)
 *
 * This structure contains all parsed Type 3 info items from the BGL file.
 * Each field corresponds to a specific info code (BGL_INFO_*).
 *
 * Memory ownership:
 * - All string pointers are malloc'd and must be freed by the caller
 * - When using bgl_reader, these are managed automatically
 *
 * Naming note:
 * - "info" = Collection/aggregation of all info fields parsed from Type 3 blocks
 */
typedef struct {
    char *title;            /**< Dictionary title (code 0x01) */
    char *author;           /**< Author (code 0x02) */
    char *email;            /**< Author email (code 0x03) */
    char *copyright;        /**< Copyright information (code 0x04) */
    char *description;      /**< Description (code 0x09) */
    char *source_lang;      /**< Source language name (code 0x07) */
    char *target_lang;      /**< Target language name (code 0x08) */
    char *source_charset;   /**< Source charset (code 0x1a) */
    char *target_charset;   /**< Target charset (code 0x1b) */
    bool utf8_mode;         /**< UTF-8 mode flag (from FLAGS code 0x11) */
    int entry_count;        /**< Number of entries (from NUM_ENTRIES code 0x0c or actual count) */
} bgl_info;

// ============================================================
// Function Declarations
// ============================================================

/**
 * @brief Parse a single Type 3 info block and update bgl_info structure
 * @param data Block data (Type 3 info block format)
 * @param data_size Data length
 * @param info Info structure to update
 * @return 0 on success, -1 on failure
 *
 * This function parses ONE Type 3 block containing a SINGLE metadata item
 * and directly updates the corresponding field in the bgl_info structure.
 *
 * Memory management:
 * - String fields are malloc'd and owned by bgl_info
 * - Caller is responsible for freeing bgl_info fields when done
 * - Charset fields point to static strings (do not free)
 */
int bgl_parse_info_field(const uint8_t *data, size_t data_size, bgl_info *info);

/**
 * @brief Convert all text fields in info to UTF-8
 * @param info Info structure to convert
 * @param encoding Source encoding (e.g., "CP936", "CP1252")
 * @return 0 on success, -1 on failure
 *
 * This function should be called after bgl_load_info completes,
 * when target_charset is known. It converts all text fields
 * (title, author, email, copyright, description) from their original
 * encoding to UTF-8.
 *
 * Memory management:
 * - Old strings are freed
 * - New UTF-8 strings are allocated
 */
int bgl_convert_info_to_utf8(bgl_info *info, const char *encoding);

/**
 * @brief Free all dynamically allocated fields in bgl_info
 * @param info Info structure to free (the struct itself is NOT freed)
 *
 * Frees all malloc'd string fields and resets them to NULL.
 * Charset fields (source_charset, target_charset) point to static
 * strings and are only set to NULL, not freed.
 */
void bgl_free_info(bgl_info *info);

#ifdef __cplusplus
}
#endif

#endif /* bgl_info_h */
