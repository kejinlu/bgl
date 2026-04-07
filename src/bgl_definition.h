//
//  bgl_definition.h
//  libud
//
//  Created by kejinlu on 2026-02-02
//

#ifndef bgl_definition_h
#define bgl_definition_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Data Structures
// ============================================================

/**
 * @brief Parsed definition fields from BGL entry
 *
 * This structure contains the parsed fields from definition data,
 * separating pure definition text from metadata fields embedded
 * within the definition.
 *
 * Memory ownership:
 * - All string pointers are malloc'd and must be freed by the caller
 * - part_of_speech points to static memory, do not free
 * - Use bgl_free_definition() to free all fields
 */
typedef struct {
    char *body;             /**< Pure definition text (UTF-8, without control codes) */
    char *title;            /**< Entry title (0x18 field, UTF-8) - displayed form of the word */
    char *title_trans;      /**< Title transcription (0x28 field, UTF-8) - romanization/translation */
    char *transcription;    /**< Transcription (0x50 field, UTF-8) - pronunciation guide */
    char *field_1a;         /**< Unknown field (0x1A, UTF-8) - seen in Hebrew dictionaries */
    const char *part_of_speech; /**< Part-of-speech string (static memory, points to bgl_pos_*), NULL if none */
} bgl_definition;

// ============================================================
// Function Declarations
// ============================================================

/**
 * @brief Parse definition fields (remove control codes and extract metadata)
 * @param data Definition data (raw bytes)
 * @param data_size Data length
 * @param source_encoding Source language encoding (for decoding title, title_trans, etc.)
 * @param target_encoding Target encoding for definition text (usually "UTF-8")
 * @param default_encoding Default fallback encoding (for charset tags without explicit encoding)
 * @param definition Output: parsed definition fields
 * @return 0 on success, -1 on failure
 *
 * Parses special fields within definition data:
 * - 0x02: Part-of-speech marker
 *   Format: \x02 + <1 byte code> (removed from definition, stored in fields->part_of_speech)
 * - 0x18: Entry title (displayed title)
 *   Format: \x18 + <1 byte length> + <title content>
 * - 0x28: Title transcription (romanization/translation)
 *   Format: \x28 + <2 bytes length, big-endian> + <HTML text>
 * - 0x1A: Unknown field (Hebrew dictionaries)
 *   Format: \x1A + <1 byte length> + <content>
 * - 0x50-0x5F: Transcription fields
 *   Format: \x5X + <1 byte length> + <content>
 * - 0x40-0x4F: Numeric/text fields
 *   Format: \x4X + <1 byte> + <text>
 *
 * The pure definition text (without control codes) is returned
 * in fields->definition.
 *
 * Encoding handling:
 * - definition: Decoded with target_encoding (supports charset tags like <charset c=U>)
 * - title, title_trans, transcription: Decoded with source_encoding
 * - default_encoding: Fallback encoding for charset tags (e.g., "CP1252")
 *
 * Memory management:
 * - All string fields in fields are malloc'd except part_of_speech (static memory)
 * - Caller must call bgl_free_definition() to free
 */
int bgl_parse_definition(const uint8_t *data, size_t data_size,
                            const char *source_encoding,
                            const char *target_encoding,
                            const char *default_encoding,
                            bgl_definition *definition);

/**
 * @brief Free definition fields
 * @param definition Definition to free
 *
 * Frees all malloc'd strings within the definition structure.
 * After calling this function, all string pointers in definition
 * will be set to NULL.
 */
void bgl_free_definition(bgl_definition *definition);

/**
 * @brief Format definition fields into a single HTML string
 * @param definition Definition to format
 * @return Newly allocated formatted string (caller must free), or NULL on failure
 *
 * Formats definition fields in the following order:
 * 1. POS + Title (if present): <span class="bgl-pos">POS</span> title<br>
 * 2. Title Trans (if present): title_trans<br>
 * 3. Transcription (if present): <span class="bgl-transcription">[transcription]</span><br>
 * 4. Body (if present): body text
 *
 * The trailing <br> is removed if body is empty.
 * Uses semantic HTML classes that can be styled with CSS.
 *
 * Note: The caller must call free() on the returned string when done.
 */
char *bgl_format_definition(const bgl_definition *definition);

#ifdef __cplusplus
}
#endif

#endif /* bgl_definition_h */
