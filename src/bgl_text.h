//
//  bgl_text.h
//  libud
//
//  Created by kejinlu on 2026-02-02
//

#ifndef bgl_text_h
#define bgl_text_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Function Declarations
// ============================================================

/**
 * @brief Decode text with charset tags
 * @param data Input data (raw bytes)
 * @param data_size Input data length
 * @param default_encoding Default encoding (e.g., "CP1252", "UTF-8")
 * @param source_encoding Source language encoding (used for c=K and c=E)
 * @return Newly allocated UTF-8 string (must be freed by caller), or NULL on failure
 *
 * Supported charset tags:
 * <charset c="U"> - UTF-8 encoding
 * <charset c="K"> - Source language encoding
 * <charset c="E"> - Source language encoding
 * <charset c="G"> - GBK encoding
 * <charset c="T"> - Babylon character references (e.g., 00E6;)
 *
 * Note: The caller must call free() on the returned string when done.
 */
char *bgl_decode_charset_tags(const uint8_t *data, size_t data_size,
                                 const char *default_encoding,
                                 const char *source_encoding);

/**
 * @brief Simple text decoding (without charset tags)
 * @param data Input data
 * @param data_size Input data length
 * @param encoding Encoding name
 * @return Newly allocated UTF-8 string (must be freed by caller), or NULL on failure
 *
 * Note: The caller must call free() on the returned string when done.
 */
char *bgl_decode_text(const uint8_t *data, size_t data_size,
                        const char *encoding);

/**
 * @brief HTML entity decoding options
 */
typedef enum {
    /** Default: Strip HTML tags and decode entities */
    BGL_HTML_STRIP,
    /** Keep HTML tags, only decode entities */
    BGL_HTML_KEEP_TAGS,
} bgl_html_option;

/**
 * @brief Decode HTML entities using libxml2
 * @param inout Pointer to string pointer (must be NULL or heap-allocated)
 * @param option HTML processing option (BGL_HTML_STRIP or BGL_HTML_KEEP_TAGS)
 * @return 0 on success, -1 on error
 *
 * Supported formats:
 * - &#123;   (decimal character reference)
 * - &#x1A5;  (hexadecimal character reference)
 * - &amp;    (named entity: amp, lt, gt, quot, apos, nbsp, etc.)
 *
 * Options:
 * - BGL_HTML_STRIP: Remove HTML tags and decode entities (for headwords)
 * - BGL_HTML_KEEP_TAGS: Keep HTML tags, only decode entities (for definitions)
 *
 * If no HTML entities are found, string remains unchanged.
 * If processing is needed, allocates new string, frees old, updates *inout.
 *
 * Unknown entities (e.g., &csdot; found in some Babylon dictionaries)
 * are preserved as-is.
 *
 * IMPORTANT: *inout must be NULL or heap-allocated (malloc, strdup, etc.)
 *    Passing string literals or stack arrays will cause crashes!
 *
 * Usage:
 *   char *str = strdup("Hello &amp; World");
 *   bgl_decode_html_entities(&str, BGL_HTML_KEEP_TAGS);
 *   printf("%s\n", str);
 *   free(str);
 */
int bgl_decode_html_entities(char **inout, bgl_html_option option);

/**
 * @brief Strip dollar sign indexes from BGL words
 * @param inout Pointer to string pointer (must be NULL or heap-allocated)
 * @return 0 on success, -1 on error
 *
 * Removes Babylon-specific dollar sign patterns:
 * - $<digits>$ pattern (e.g., $1$, $2$, $4$)
 * - Consecutive dollar signs (e.g., $$, $$$$, $$$$$$)
 *
 * If no dollar patterns found, string remains unchanged.
 *
 * Examples:
 * - "make do$4$/make" → "make do/make"
 * - "potere$1$<BR>" → "potere<BR>"
 * - "test$$$$" → "test"
 * - "Ihre$1$Ihres" → "IhreIhres"
 *
 * IMPORTANT: *inout must be NULL or heap-allocated
 */
int bgl_strip_dollar_indexes(char **inout);

/**
 * @brief Remove BGL control characters from text
 * @param inout Pointer to string pointer (must be NULL or heap-allocated)
 * @return 0 on success, -1 on error
 *
 * Removes Babylon-specific control characters while preserving whitespace:
 * - Removes: \x00-\x08, \x0c, \x0e-\x1f
 * - Preserves: \x09 (tab), \x0a (LF), \x0b (VT), \x0d (CR)
 *
 * If no control characters found, string remains unchanged.
 *
 * IMPORTANT: *inout must be NULL or heap-allocated
 */
int bgl_remove_control_chars(char **inout);

/**
 * @brief Remove newlines from text (replace with spaces)
 * @param inout Pointer to string pointer (must be NULL or heap-allocated)
 * @return 0 on success, -1 on error
 *
 * Replaces all newline characters (\r, \n, \r\n) with single spaces.
 * Consecutive newlines are collapsed into a single space.
 *
 * If no newlines found, string remains unchanged.
 *
 * IMPORTANT: *inout must be NULL or heap-allocated
 */
int bgl_remove_newlines(char **inout);

/**
 * @brief Remove slash before word at string start or after whitespace
 * @param inout Pointer to string pointer (must be NULL or heap-allocated)
 * @return 0 on success, -1 on error
 *
 * Removes "/" character that appears:
 * - At the beginning of the string: "/word" → "word"
 * - After whitespace: " /word" → " word"
 * - After tabs/spaces: "\t/word" → "\tword"
 *
 * Pattern: r"(^|\s)/(\w)" → replace with r"\1\2" (remove the /)
 *
 * If no matching pattern found, string remains unchanged.
 *
 * IMPORTANT: *inout must be NULL or heap-allocated
 */
int bgl_strip_slash_alt_key(char **inout);

/**
 * @brief Fix image links by removing BGL control characters
 * @param inout Pointer to string pointer (must be NULL or heap-allocated)
 * @return 0 on success, -1 on error
 *
 * Removes BGL-specific control characters used in image tags:
 * - \x1e (Record Separator, RS)
 * - \x1f (Unit Separator, US)
 *
 * These characters are used to enclose src attribute values in BGL image tags:
 * <IMG border='0' src='\x1e6B6C56EC.png\x1f' width='9' height='8'>
 *
 * They are not part of the actual image filename and can be safely removed.
 * These control characters are useless in HTML text.
 *
 * If no control characters found, string remains unchanged.
 *
 * Example:
 * - "<IMG src='\\x1eimage.png\\x1f'>" → "<IMG src='image.png'>"
 *
 * IMPORTANT: *inout must be NULL or heap-allocated
 */
int bgl_fix_img_links(char **inout);

/**
 * @brief Normalize newlines to Unix style and remove consecutive newlines
 * @param inout Pointer to string pointer (must be NULL or heap-allocated)
 * @return 0 on success, -1 on error
 *
 * Converts all newlines to Unix style (\n) and removes consecutive newlines.
 * Matches the pattern [\r\n]+ → replace with \n
 *
 * This handles:
 * - \r\n (Windows) → \n
 * - \r (old Mac) → \n
 * - \n (Unix) → \n
 * - Consecutive newlines (e.g., \n\n, \r\r\n) → single \n
 *
 * If no newlines found, string remains unchanged.
 *
 * IMPORTANT: *inout must be NULL or heap-allocated
 */
int bgl_normalize_newlines(char **inout);

/**
 * @brief Strip whitespace from both ends of a string
 * @param inout Pointer to string pointer (must be NULL or heap-allocated)
 * @return 0 on success, -1 on error
 *
 * Removes leading and trailing whitespace characters:
 * - Space (' ')
 * - Tab ('\t')
 * - Newline ('\n', '\r')
 * - Vertical tab ('\v')
 * - Form feed ('\f')
 *
 * This is equivalent to Python's str.strip() method.
 *
 * If no leading/trailing whitespace found, string remains unchanged.
 *
 * Examples:
 * - "  hello  " → "hello"
 * - "\t\nword\n\r" → "word"
 * - "no-change" → "no-change"
 * - "   " → "" (empty string)
 *
 * IMPORTANT: *inout must be NULL or heap-allocated
 */
int bgl_strip(char **inout);

/**
 * @brief Strip HTML tags from string
 * @param inout Pointer to string pointer (must be NULL or heap-allocated)
 * @return 0 on success, -1 on error
 *
 * Removes all HTML tags, replacing them with spaces.
 * Preserves plain text content between tags.
 *
 * If no HTML tags found, string remains unchanged.
 *
 * Examples:
 * - "<b>Hello</b> World" → "Hello  World"
 * - "NoTags" → "NoTags"
 *
 * IMPORTANT: *inout must be NULL or heap-allocated
 */
int bgl_strip_html_tags(char **inout);

// ============================================================
// Helper Macros - REMOVED
// ============================================================
// Note: With the new char** API, macros are no longer needed.
// Simply call functions directly:
//   char *str = get_text();
//   bgl_strip(&str);
//   bgl_normalize_newlines(&str);
//   free(str);

#ifdef __cplusplus
}
#endif

#endif /* bgl_text_h */
