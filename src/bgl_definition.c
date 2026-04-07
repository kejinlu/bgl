//
//  bgl_definition.c
//  libud
//
//  Created by kejinlu on 2026-02-02
//

#include "bgl_definition.h"
#include "bgl_pos.h"
#include "bgl_text.h"
#include "bgl_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// Control Code Constants
// ============================================================

/** Definition field separator */
#define BGL_DEF_FIELD_SEPARATOR     0x14

/** Space character (used to detect \x14 in article body) */
#define BGL_SPACE_CHAR              0x20

/** Part-of-speech field */
#define BGL_DEF_FIELD_POS           0x02

/** Entry title field */
#define BGL_DEF_FIELD_TITLE         0x18

/** Title transcription field */
#define BGL_DEF_FIELD_TITLE_TRANS   0x28

/** Unknown field (Hebrew dictionaries) */
#define BGL_DEF_FIELD_UNKNOWN_1A    0x1A

/** Part-of-speech code range */
#define BGL_POS_CODE_MIN            0x30
#define BGL_POS_CODE_MAX            0x47

/** Transcription field 50 */
#define BGL_DEF_FIELD_TRANS_50      0x50

/** Transcription field 60 */
#define BGL_DEF_FIELD_TRANS_60      0x60

/** Numeric/text field range */
#define BGL_DEF_FIELD_NUMERIC_MIN   0x40
#define BGL_DEF_FIELD_NUMERIC_MAX   0x4F

/** Base for calculating fixed-length field size */
#define BGL_DEF_FIELD_NUMERIC_BASE  0x3F

// ============================================================
// Function Implementations
// ============================================================

int bgl_parse_definition(const uint8_t *data, size_t data_size,
                            const char *source_encoding,
                            const char *target_encoding,
                            const char *default_encoding,
                            bgl_definition *definition) {
    if (!data || data_size == 0 || !source_encoding || !target_encoding || !definition) {
        return -1;
    }

    // Initialize all fields
    memset(definition, 0, sizeof(bgl_definition));

    // Step 1: Find the separator (signals beginning of definition fields)
    // According to pyglossary, separator followed by space is part of article
    size_t fields_start = data_size;  // Default: no fields, entire data is definition
    for (size_t i = 0; i < data_size - 1; i++) {
        if (data[i] == BGL_DEF_FIELD_SEPARATOR && data[i + 1] != BGL_SPACE_CHAR) {
            fields_start = i;
            break;
        }
    }

    // Step 2: Extract definition (everything before separator)
    const uint8_t *def_data = data;
    size_t def_len = fields_start;

    // Decode definition with target_encoding (supports charset tags)
    // target_encoding is the fallback for text without explicit charset tags
    // because definition body is target language content (e.g., Chinese in an
    // English-Chinese dictionary). Using default_encoding (typically CP1252) would
    // produce garbled output for non-Latin target languages.
    if (def_len > 0) {
        definition->body = bgl_decode_charset_tags(def_data, def_len, target_encoding,
                                                       source_encoding);
        if (definition->body == NULL) {
            // Fallback: try simple decode
            definition->body = bgl_decode_text(def_data, def_len, target_encoding);
            if (definition->body == NULL) {
                // Last resort: copy as-is (might not be UTF-8)
                definition->body = (char *)malloc(def_len + 1);
                if (definition->body) {
                    memcpy(definition->body, def_data, def_len);
                    definition->body[def_len] = '\0';
                }
            }
        }
    } else {
        // Empty definition
        definition->body = (char *)malloc(1);
        if (definition->body) {
            definition->body[0] = '\0';
        }
    }

    // Step 3: Parse fields after separator (if any)
    size_t i = fields_start + 1;  // Skip separator
    while (i < data_size) {
        uint8_t code = data[i++];  // Read field code and increment

        // Part-of-speech field: <field_code 0x02> <pos_code>
        if (code == BGL_DEF_FIELD_POS) {
            if (i >= data_size) {
                break;  // Incomplete field
            }
            uint8_t pos_code = data[i];  // Peek at next byte, don't increment yet

            // Check if valid POS code (0x30-0x47)
            if (pos_code >= BGL_POS_CODE_MIN && pos_code <= BGL_POS_CODE_MAX) {
                i++;  // Now increment to skip pos_code
                // Extract POS
                definition->part_of_speech = bgl_pos_abbr_by_code(pos_code);
                if (!definition->part_of_speech || definition->part_of_speech[0] == '\0') {
                    definition->part_of_speech = bgl_pos_name_by_code(pos_code);
                }
            } else {
                // Not a valid POS code, don't skip it
                // Next iteration will retry this byte as a new field code
                continue;
            }

        // Entry title field: <field_code 0x18> <1 byte length> <title>
        } else if (code == BGL_DEF_FIELD_TITLE) {
            if (i + 1 > data_size) {
                break;  // Incomplete field
            }
            uint8_t len = data[i++];  // Read length

            if (len == 0 || i + len > data_size) {
                continue;  // Empty or invalid length
            }

            // Decode title with source_encoding as default encoding (supports charset tags)
            // Matches pyglossary's processDefi flow (u_title)
            definition->title = bgl_decode_charset_tags(data + i, len, source_encoding,
                                                           source_encoding);

            i += len;  // Skip title data

        // Title transcription field: <field_code 0x28> <2 bytes length, big-endian> <HTML text>
        } else if (code == BGL_DEF_FIELD_TITLE_TRANS) {
            if (i + 2 > data_size) {
                break;
            }
            uint16_t len = bgl_read_uint16_be(data + i);
            i += 2;

            if (len == 0 || i + len > data_size) {
                continue;
            }

            // Decode title_trans with source_encoding (charset tags may be present)
            definition->title_trans = bgl_decode_charset_tags(data + i, len, source_encoding,
                                                                 source_encoding);

            i += len;  // Skip title_trans data

        // Unknown field 1a (Hebrew dictionaries): <field_code 0x1A> <1 byte length> <content>
        } else if (code == BGL_DEF_FIELD_UNKNOWN_1A) {
            if (i + 1 > data_size) {
                break;
            }
            uint8_t len = data[i++];  // Read length

            if (len == 0 || i + len > data_size) {
                continue;
            }

            // Decode field_1a with source_encoding
            // Note: No additional processing needed (only used for debug output in pyglossary)
            definition->field_1a = bgl_decode_text(data + i, len, source_encoding);
            i += len;  // Skip field_1a data

        // Transcription fields:
        // 0x50: <field_code 0x50> <trans_code> <1 byte length> <data>
        // 0x60: <field_code 0x60> <trans_code> <2 bytes length> <data>
        } else if (code == BGL_DEF_FIELD_TRANS_50 || code == BGL_DEF_FIELD_TRANS_60) {
            if (i >= data_size) {
                break;
            }
            uint8_t trans_code = data[i++];  // Read transcription type code

            // Only process transcription when code == 0x1B
            // Other codes (0x10, 0x18, etc.) are not valid text data
            if (trans_code != 0x1B) {
                // Skip invalid transcription data
                if (code == BGL_DEF_FIELD_TRANS_50) {
                    // 0x50: 1 byte length
                    if (i >= data_size) {
                        break;
                    }
                    uint8_t len = data[i++];
                    i += len;  // Skip data
                } else {
                    // 0x60: 2 bytes length
                    if (i + 2 > data_size) {
                        break;
                    }
                    uint16_t len = bgl_read_uint16_be(data + i);
                    i += 2 + len;  // Skip length bytes and data
                }
                continue;
            }

            // Read length (different for 0x50 vs 0x60)
            uint16_t len;
            if (code == BGL_DEF_FIELD_TRANS_50) {
                // 0x50: 1 byte length
                if (i >= data_size) {
                    break;
                }
                len = data[i++];
            } else {
                // 0x60: 2 bytes length (big-endian)
                if (i + 2 > data_size) {
                    break;
                }
                len = bgl_read_uint16_be(data + i);
                i += 2;
            }

            if (len == 0 || i + len > data_size) {
                continue;
            }

            // Store transcription (only the first one)
            if (!definition->transcription) {
                // Decode with source_encoding (supports charset tags)
                // Matches pyglossary's processDefi flow (u_transcription_50/60)
                definition->transcription = bgl_decode_charset_tags(data + i, len,
                                                                        source_encoding,
                                                                        source_encoding);
            }
            i += len;  // Skip transcription data

        // Numeric/text fields: <field_code 0x40-0x4F> (length = code - 0x3F) <text>
        } else if (code >= BGL_DEF_FIELD_NUMERIC_MIN && code <= BGL_DEF_FIELD_NUMERIC_MAX) {
            uint8_t len = code - BGL_DEF_FIELD_NUMERIC_BASE;

            if (i + len > data_size) {
                break;
            }

            // Skip these fields (usually digits or special formatting)
            i += len;

        } else {
            // Unknown field code, skip
            // (already incremented past the code byte)
        }
    }

    return 0;
}

void bgl_free_definition(bgl_definition *definition) {
    if (!definition) {
        return;
    }

    if (definition->body) {
        free(definition->body);
        definition->body = NULL;
    }
    if (definition->title) {
        free(definition->title);
        definition->title = NULL;
    }
    if (definition->title_trans) {
        free(definition->title_trans);
        definition->title_trans = NULL;
    }
    if (definition->transcription) {
        free(definition->transcription);
        definition->transcription = NULL;
    }
    if (definition->field_1a) {
        free(definition->field_1a);
        definition->field_1a = NULL;
    }
}

char *bgl_format_definition(const bgl_definition *definition) {
    if (!definition) {
        return NULL;
    }

    // Helper to get string length safely
    #define STRLEN(s) ((s) ? strlen(s) : 0)

    // Calculate total required length
    size_t total_len = 0;

    // 1. POS + Title part
    bool has_pos_title = (definition->part_of_speech || definition->title);
    if (has_pos_title) {
        if (definition->part_of_speech) {
            // <span class="bgl-pos">POS</span>
            total_len += strlen("<span class=\"bgl-pos\">") + STRLEN(definition->part_of_speech) + strlen("</span>");
        }
        if (definition->title) {
            // " " before title if POS exists
            if (definition->part_of_speech) {
                total_len += 1;  // space
            }
            total_len += STRLEN(definition->title);
        }
        total_len += strlen("<br>");  // line break after POS/title
    }

    // 2. Title Trans part
    if (definition->title_trans) {
        total_len += STRLEN(definition->title_trans) + strlen("<br>");
    }

    // 3. Transcription part
    if (definition->transcription) {
        // <span class="bgl-transcription">[transcription]</span>
        total_len += strlen("<span class=\"bgl-transcription\">[") +
                      STRLEN(definition->transcription) +
                      strlen("]</span><br>");
    }

    // 4. Body part
    if (definition->body) {
        total_len += STRLEN(definition->body);
    }

    // Allocate memory (+1 for null terminator)
    char *result = (char *)malloc(total_len + 1);
    if (!result) {
        return NULL;
    }

    // Build the formatted string
    size_t pos = 0;

    // 1. POS + Title part
    if (has_pos_title) {
        if (definition->part_of_speech) {
            pos += sprintf(result + pos, "<span class=\"bgl-pos\">%s</span>", definition->part_of_speech);
        }
        if (definition->title) {
            if (definition->part_of_speech) {
                pos += sprintf(result + pos, " ");
            }
            pos += sprintf(result + pos, "%s", definition->title);
        }
        pos += sprintf(result + pos, "<br>");
    }

    // 2. Title Trans part
    if (definition->title_trans) {
        pos += sprintf(result + pos, "%s<br>", definition->title_trans);
    }

    // 3. Transcription part
    if (definition->transcription) {
        pos += sprintf(result + pos, "<span class=\"bgl-transcription\">[%s]</span><br>", definition->transcription);
    }

    // 4. Body part
    if (definition->body) {
        pos += sprintf(result + pos, "%s", definition->body);
    }

    result[pos] = '\0';

    // Remove trailing <br> if body is empty (matches pyglossary's removesuffix("<br"))
    if (!definition->body && pos > 4) {
        char *p = result + pos - 4;
        if (strcmp(p, "<br>") == 0) {
            *p = '\0';
        }
    }

    #undef STRLEN
    return result;
}
