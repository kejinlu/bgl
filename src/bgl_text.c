//
//  bgl_text.c
//  libud
//
//  Created by kejinlu on 2026-02-02
//

#include "bgl_text.h"
#include "bgl_util.h"
#include "bgl_port.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iconv.h>
#include <libxml/HTMLparser.h>

// ============================================================
// Constants
// ============================================================

#define CHARSET_TAG_OPEN "<charset"
#define CHARSET_TAG_CLOSE "</charset"
#define MAX_BUFFER_SIZE (10 * 1024 * 1024)  // 10MB
#define MAX_ENCODING_DEPTH 32               // Maximum nesting depth
#define MAX_SEGMENTS 256                    // Maximum number of text segments

/** Charset tag lengths (for fallback when '>' not found) */
#define CHARSET_CLOSE_TAG_LEN 10            // </charset> length
#define CHARSET_OPEN_TAG_MIN_LEN 15         // <charset c="X"> length (with quotes)

// ============================================================
// Internal Data Structures
// ============================================================

/**
 * @brief Text segment with encoding information
 */
typedef struct {
    const uint8_t *data;       /**< Pointer to segment data in original buffer */
    size_t len;                /**< Segment length */
    const char *encoding;      /**< Encoding for this segment */
} text_segment_t;

/**
 * @brief Encoding stack for nested charset tags
 */
typedef struct {
    const char *encodings[MAX_ENCODING_DEPTH]; /**< Stack of encoding names */
    int depth;                                  /**< Current stack depth */
} encoding_stack_t;

// ============================================================
// Internal Helper Functions - Forward Declarations
// ============================================================

static int decode_babylon_char_ref(const uint8_t *hex_str, size_t hex_len,
                                   char *utf8_out, size_t *out_len);

static int bgl_parse_char_ref(const char *hex_string, size_t str_len,
                                  uint32_t *code_out);

// ============================================================
// Internal Helper Functions
// ============================================================

/**
 * @brief Initialize encoding stack
 */
static void encoding_stack_init(encoding_stack_t *stack, const char *default_encoding) {
    stack->depth = 0;
    if (default_encoding) {
        stack->encodings[stack->depth] = default_encoding;
        stack->depth++;
    }
}

/**
 * @brief Push encoding onto stack
 */
static int encoding_stack_push(encoding_stack_t *stack, const char *encoding) {
    if (!encoding || stack->depth >= MAX_ENCODING_DEPTH) {
        return -1;
    }
    stack->encodings[stack->depth] = encoding;
    stack->depth++;
    return 0;
}

/**
 * @brief Pop encoding from stack
 */
static void encoding_stack_pop(encoding_stack_t *stack) {
    if (stack->depth > 1) {
        stack->depth--;
    }
}

/**
 * @brief Get current encoding (top of stack)
 */
static const char *encoding_stack_top(encoding_stack_t *stack) {
    if (stack->depth == 0) {
        return NULL;
    }
    return stack->encodings[stack->depth - 1];
}

/**
 * @brief Get charset encoding by type character
 * @param type Charset type (u, k, e, g, t)
 * @param source_encoding Source language encoding
 * @return Encoding string (static, do not free)
 */
static const char *get_charset_encoding(char type, const char *source_encoding) {
    switch (type) {
        case 'u': return "UTF-8";
        case 'k': // Source encoding
        case 'e': return source_encoding ? source_encoding : "CP1252";
        case 'g': return "GBK";
        case 't': return "BABYLON-REF";  // Special marker for Babylon character references
        default:  return NULL;
    }
}

/**
 * @brief Check if string matches a hex reference pattern (4 hex digits)
 */
static bool is_hex_reference(const uint8_t *data, size_t len) {
    if (len < 4 || len > 6) {  // Allow 4-5 hex digits plus optional semicolon
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        uint8_t c = data[i];
        if (c == ';') {
            // Check if all previous characters are hex
            return (i >= 4);  // At least 4 hex digits before semicolon
        }
        if (!isxdigit(c)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Decode Babylon character reference segment
 * Converts semicolon-separated hex values (e.g., "00E6;00E7;") to UTF-8 string
 */
static char *decode_babylon_ref_segment(const uint8_t *data, size_t len, size_t *out_len) {
    // Allocate buffer for output (each reference produces 1-4 UTF-8 bytes)
    size_t max_output = (len / 5) * 4 + 1;  // Approximate: 5 chars per ref, max 4 UTF-8 bytes
    char *result = (char *)malloc(max_output);
    if (!result) {
        return NULL;
    }

    size_t result_pos = 0;
    size_t start = 0;

    while (start < len) {
        // Find next semicolon
        size_t end = start;
        while (end < len && data[end] != ';') {
            end++;
        }

        if (end > start) {
            // Parse hex value
            char utf8_buf[4];
            size_t utf8_len = 0;

            if (decode_babylon_char_ref(data + start, end - start, utf8_buf, &utf8_len) == 0) {
                // Append to result
                if (result_pos + utf8_len < max_output) {
                    memcpy(result + result_pos, utf8_buf, utf8_len);
                    result_pos += utf8_len;
                }
            }
        }

        start = end + 1;  // Skip semicolon
    }

    result[result_pos] = '\0';
    *out_len = result_pos;
    return result;
}

/**
 * @brief Encoding conversion using iconv
 */
static char *convert_encoding_iconv(const char *to_encoding, const char *from_encoding,
                                   const uint8_t *input, size_t input_len, size_t *output_len) {
    if (!input || input_len == 0) {
        *output_len = 0;
        return bgl_strdup("");
    }

    iconv_t cd = iconv_open(to_encoding, from_encoding);
    if (cd == (iconv_t)-1) {
        return NULL;
    }

    size_t outbuf_size = input_len * 4;
    char *outbuf = (char *)malloc(outbuf_size);
    if (!outbuf) {
        iconv_close(cd);
        return NULL;
    }

    char *inptr = (char *)input;
    char *outptr = outbuf;
    size_t inbytes_left = input_len;
    size_t outbytes_left = outbuf_size;

    size_t ret = iconv(cd, &inptr, &inbytes_left, &outptr, &outbytes_left);
    iconv_close(cd);

    if (ret == (size_t)-1) {
        free(outbuf);
        return NULL;
    }

    *output_len = outbuf_size - outbytes_left;

    // Reallocate to actual size
    char *result = (char *)realloc(outbuf, *output_len + 1);
    if (!result) {
        result = outbuf;
    }
    result[*output_len] = '\0';

    return result;
}

/**
 * @brief Find string (case-insensitive)
 */
static const uint8_t *find_str_case_insensitive(const uint8_t *haystack, size_t haystack_len,
                                                const char *needle, size_t *pos) {
    size_t needle_len = strlen(needle);
    if (needle_len == 0 || needle_len > haystack_len) {
        return NULL;
    }

    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        bool match = true;
        for (size_t j = 0; j < needle_len; j++) {
            if (tolower(haystack[i + j]) != tolower((uint8_t)needle[j])) {
                match = false;
                break;
            }
        }
        if (match) {
            if (pos) {
                *pos = i;
            }
            return haystack + i;
        }
    }

    return NULL;
}

/**
 * @brief Parse charset tag type
 */
static char parse_charset_tag_type(const uint8_t *tag_start, size_t tag_len) {
    // Find c="
    const uint8_t *c_pos = find_str_case_insensitive(tag_start, tag_len, "c=", NULL);
    if (!c_pos) return '\0';

    // Skip c= and possible quote
    const uint8_t *type_start = c_pos + 2;
    if (*type_start == '"' || *type_start == '\'') {
        type_start++;
    }

    if (type_start >= tag_start + tag_len) {
        return '\0';
    }

    return tolower(*type_start);
}

/**
 * @brief Decode Babylon character reference (e.g., 00E6;)
 */
static int decode_babylon_char_ref(const uint8_t *hex_str, size_t hex_len, char *utf8_out, size_t *out_len) {
    // Convert to string
    char *hex_str_copy = (char *)malloc(hex_len + 1);
    if (!hex_str_copy) return -1;
    memcpy(hex_str_copy, hex_str, hex_len);
    hex_str_copy[hex_len] = '\0';

    uint32_t code_point;
    if (bgl_parse_char_ref(hex_str_copy, hex_len, &code_point) != 0) {
        free(hex_str_copy);
        return -1;
    }
    free(hex_str_copy);

    *out_len = bgl_codepoint_to_utf8(code_point, utf8_out);
    return (*out_len > 0) ? 0 : -1;
}

// ============================================================
// Function Implementations
// ============================================================

/**
 * @brief Parse Babylon character reference (e.g., "00E6;")
 * @param hex_string Hexadecimal string (e.g., "00E6")
 * @param str_len String length
 * @param code_out Output: Unicode code point
 * @return 0 on success, -1 on failure
 */
static int bgl_parse_char_ref(const char *hex_string, size_t str_len, uint32_t *code_out) {
    if (!hex_string || str_len == 0 || !code_out) {
        return -1;
    }

    uint32_t code = 0;
    for (size_t i = 0; i < str_len; i++) {
        char c = hex_string[i];
        if (c == ';') break;

        code <<= 4;
        if (c >= '0' && c <= '9') {
            code |= (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            code |= (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            code |= (c - 'A' + 10);
        } else {
            return -1;  // Invalid hex character
        }
    }

    *code_out = code;
    return 0;
}

char *bgl_decode_text(const uint8_t *data, size_t data_size,
                      const char *encoding) {
    if (!data || data_size == 0 || !encoding) {
        return NULL;
    }

    size_t out_len = 0;
    return convert_encoding_iconv("UTF-8", encoding, data, data_size, &out_len);
}

/**
 * @brief Find length of charset tag (from start to '>')
 * @param tag_start Start of tag (pointing to "<charset")
 * @param max_len Maximum search length
 * @return Length of tag including '>', or 0 if not found
 */
static size_t find_tag_len(const uint8_t *tag_start, size_t max_len) {
    for (size_t i = 0; i < max_len; i++) {
        if (tag_start[i] == '>') {
            return i + 1;
        }
    }
    return 0;
}

/**
 * @brief Parse charset tags and split text into segments
 *
 * This function implements full charset tag parsing similar to pyglossary:
 * - Handles multiple charset tags in a single text
 * - Supports nested tags using encoding stack
 * - Supports all charset types: U (UTF-8), K/E (source), G (GBK), T (Babylon ref)
 *
 * Algorithm:
 * 1. Scan through input data finding <charset> and </charset> tags
 * 2. Maintain encoding stack to handle nested tags
 * 3. Split text into segments, each with its own encoding
 * 4. Decode each segment to UTF-8
 * 5. Concatenate all segments
 */
char *bgl_decode_charset_tags(const uint8_t *data, size_t data_size,
                                 const char *default_encoding,
                                 const char *source_encoding) {
    if (!data || data_size == 0 || !default_encoding) {
        return NULL;
    }

    // Initialize encoding stack with default encoding
    encoding_stack_t stack;
    encoding_stack_init(&stack, default_encoding);

    // Array to store text segments
    text_segment_t segments[MAX_SEGMENTS];
    int segment_count = 0;

    size_t pos = 0;
    size_t text_start = 0;  // Start of current text segment

    while (pos < data_size && segment_count < MAX_SEGMENTS) {
        size_t open_pos;
        const uint8_t *open_tag = find_str_case_insensitive(data + pos, data_size - pos,
                                                           CHARSET_TAG_OPEN, &open_pos);

        // Check for closing tag
        size_t close_pos;
        const uint8_t *close_tag = find_str_case_insensitive(data + pos, data_size - pos,
                                                            CHARSET_TAG_CLOSE, &close_pos);

        // Determine which tag comes first
        bool has_open = (open_tag != NULL);
        bool has_close = (close_tag != NULL);
        const uint8_t *next_tag = NULL;
        bool is_closing = false;

        if (has_open && has_close) {
            if (open_pos < close_pos) {
                next_tag = open_tag;
                is_closing = false;
            } else {
                next_tag = close_tag;
                is_closing = true;
            }
        } else if (has_open) {
            next_tag = open_tag;
            is_closing = false;
        } else if (has_close) {
            next_tag = close_tag;
            is_closing = true;
        } else {
            // No more tags, add remaining text
            if (text_start < data_size) {
                segments[segment_count].data = data + text_start;
                segments[segment_count].len = data_size - text_start;
                segments[segment_count].encoding = encoding_stack_top(&stack);
                segment_count++;
            }
            break;
        }

        size_t tag_offset = next_tag - data;

        // Add text segment before this tag
        if (tag_offset > text_start) {
            segments[segment_count].data = data + text_start;
            segments[segment_count].len = tag_offset - text_start;
            segments[segment_count].encoding = encoding_stack_top(&stack);
            segment_count++;

            if (segment_count >= MAX_SEGMENTS) {
                break;
            }
        }

        if (is_closing) {
            // </charset> tag
            encoding_stack_pop(&stack);
            // Find length of closing tag
            size_t tag_len = find_tag_len(next_tag, data_size - tag_offset);
            pos = tag_offset + (tag_len > 0 ? tag_len : CHARSET_CLOSE_TAG_LEN);
        } else {
            // <charset c="X"> tag
            char tag_type = parse_charset_tag_type(next_tag, data_size - tag_offset);
            const char *new_encoding = get_charset_encoding(tag_type, source_encoding);

            if (new_encoding) {
                // Push encoding onto stack (treat "BABYLON-REF" like any other encoding)
                encoding_stack_push(&stack, new_encoding);
                size_t tag_len = find_tag_len(next_tag, data_size - tag_offset);
                pos = tag_offset + (tag_len > 0 ? tag_len : CHARSET_OPEN_TAG_MIN_LEN);
            } else {
                // Invalid charset type, skip tag
                size_t tag_len = find_tag_len(next_tag, data_size - tag_offset);
                pos = tag_offset + (tag_len > 0 ? tag_len : CHARSET_OPEN_TAG_MIN_LEN);
            }
        }
        // Update text_start after processing any tag
        text_start = pos;
    }

    // Decode each segment and concatenate
    size_t total_len = 0;
    char **decoded_parts = (char **)calloc(segment_count, sizeof(char *));
    if (!decoded_parts) {
        return NULL;
    }

    for (int i = 0; i < segment_count; i++) {
        if (strcmp(segments[i].encoding, "BABYLON-REF") == 0) {
            // Decode Babylon character references
            size_t part_len;
            decoded_parts[i] = decode_babylon_ref_segment(segments[i].data,
                                                          segments[i].len, &part_len);
            if (decoded_parts[i]) {
                total_len += part_len;
            }
        } else {
            // Decode using iconv
            size_t part_len;
            decoded_parts[i] = convert_encoding_iconv("UTF-8", segments[i].encoding,
                                                      segments[i].data,
                                                      segments[i].len, &part_len);
            if (decoded_parts[i]) {
                total_len += part_len;
            }
        }
    }

    // Allocate final result buffer
    char *final_result = (char *)malloc(total_len + 1);
    if (!final_result) {
        for (int i = 0; i < segment_count; i++) {
            free(decoded_parts[i]);
        }
        free(decoded_parts);
        return NULL;
    }

    // Concatenate all parts
    size_t offset = 0;
    for (int i = 0; i < segment_count; i++) {
        if (decoded_parts[i]) {
            size_t part_len = strlen(decoded_parts[i]);
            memcpy(final_result + offset, decoded_parts[i], part_len);
            offset += part_len;
            free(decoded_parts[i]);
        }
    }
    final_result[total_len] = '\0';

    free(decoded_parts);
    return final_result;
}

// ============================================================
// HTML Entity Decoding
// ============================================================

/**
 * @brief Check if string contains HTML entities
 */
static bool has_html_entities(const char *str) {
    if (!str) return false;
    for (const char *p = str; *p; p++) {
        if (*p == '&') {
            return true;
        }
    }
    return false;
}

/**
 * @brief Decode a single HTML entity (numeric or named)
 * @param entity String starting with '&', ending with ';'
 * @param output Output buffer (at least 8 bytes for UTF-8)
 * @return Number of bytes written, or 0 on failure
 */
static size_t decode_single_entity(const char *entity, char *output) {
    if (!entity || entity[0] != '&') return 0;

    // Find semicolon
    const char *semicolon = strchr(entity, ';');
    if (!semicolon) return 0;

    size_t entity_len = semicolon - entity + 1;

    // Numeric decimal: &#123;
    if (entity_len >= 4 && entity[1] == '#') {
        if (entity_len >= 5 && (entity[2] == 'x' || entity[2] == 'X')) {
            // Hexadecimal: &#x1A5;
            uint32_t code = 0;
            for (size_t i = 3; i < entity_len - 1; i++) {
                char c = entity[i];
                code <<= 4;
                if (c >= '0' && c <= '9') code |= (c - '0');
                else if (c >= 'a' && c <= 'f') code |= (c - 'a' + 10);
                else if (c >= 'A' && c <= 'F') code |= (c - 'A' + 10);
                else return 0;  // Invalid hex
            }
            return bgl_codepoint_to_utf8(code, output);
        } else {
            // Decimal: &#123;
            uint32_t code = 0;
            for (size_t i = 2; i < entity_len - 1; i++) {
                char c = entity[i];
                if (c >= '0' && c <= '9') {
                    code = code * 10 + (c - '0');
                } else {
                    return 0;  // Invalid decimal
                }
            }
            return bgl_codepoint_to_utf8(code, output);
        }
    }

    // Named entities (common ones)
    const char *name = entity + 1;
    size_t name_len = entity_len - 2;

    static const struct { const char *name; uint32_t code; } entities[] = {
        {"amp", '&'}, {"lt", '<'}, {"gt", '>'}, {"quot", '"'},
        {"apos", '\''}, {"nbsp", 160}, {"copy", 169}, {"reg", 174},
        {"euro", 8364}, {"mdash", 8211}, {"ldash", 8211},
        {"lsquo", 8216}, {"rsquo", 8217}, {"hellip", 8230},
    };

    for (size_t i = 0; i < sizeof(entities)/sizeof(entities[0]); i++) {
        size_t len = strlen(entities[i].name);
        if (name_len == len && bgl_strncasecmp(name, entities[i].name, len) == 0) {
            return bgl_codepoint_to_utf8(entities[i].code, output);
        }
    }

    // Unknown entity - return 0 to indicate failure
    return 0;
}

/**
 * @brief Decode HTML entities while preserving HTML tags (KEEP_TAGS option)
 */
static char *decode_entities_keep_tags(const char *input) {
    if (!input) return NULL;

    size_t input_len = strlen(input);
    char *result = (char *)malloc(input_len * 4 + 1);  // Worst case expansion
    if (!result) return NULL;

    size_t result_pos = 0;
    size_t i = 0;

    while (i < input_len) {
        if (input[i] == '&') {
            // Try to decode entity
            char utf8_buf[8];
            size_t utf8_len = decode_single_entity(input + i, utf8_buf);

            if (utf8_len > 0) {
                // Found valid entity, skip to semicolon
                const char *semicolon = strchr(input + i, ';');
                if (semicolon) {
                    memcpy(result + result_pos, utf8_buf, utf8_len);
                    result_pos += utf8_len;
                    i = (semicolon - input) + 1;
                    continue;
                }
            }
        }

        // Not an entity or failed to decode, copy as-is
        result[result_pos++] = input[i++];
    }

    result[result_pos] = '\0';

    // Shrink to actual size
    char *shrunk = (char *)realloc(result, result_pos + 1);
    if (shrunk) return shrunk;

    return result;
}

/**
 * @brief Internal implementation for HTML entity decoding
 * Returns newly allocated string or NULL on failure.
 */
static char *bgl_decode_html_entities_impl(const char *input, bgl_html_option option) {
    if (!input) {
        return NULL;
    }

    // Handle empty string
    if (input[0] == '\0') {
        return bgl_strdup("");
    }

    // KEEP_TAGS: Use custom implementation that preserves tags
    if (option == BGL_HTML_KEEP_TAGS) {
        return decode_entities_keep_tags(input);
    }

    // STRIP option: Use libxml2 (removes tags and decodes entities)
    // Use libxml2's HTML parser to decode entities
    // Create a minimal HTML document context for parsing
    htmlDocPtr doc = htmlReadDoc(
        (const xmlChar *)input,
        NULL,             // URL (not needed)
        "UTF-8",          // encoding
        HTML_PARSE_NOERROR |    // Suppress error reports
        HTML_PARSE_NOWARNING |  // Suppress warning reports
        HTML_PARSE_NONET |      // Disable network access
        HTML_PARSE_NOIMPLIED    // Don't add implied tags
    );

    if (!doc) {
        // Parsing failed - this can happen with malformed HTML
        // Fall back to returning the original string
        return bgl_strdup(input);
    }

    // Get the root element
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root) {
        xmlFreeDoc(doc);
        return bgl_strdup(input);
    }

    // Extract text content from the parsed document
    xmlBufferPtr buf = xmlBufferCreate();
    if (!buf) {
        xmlFreeDoc(doc);
        return NULL;
    }

    // Collect all text nodes
    for (xmlNodePtr node = root->children; node; node = node->next) {
        if (node->type == XML_TEXT_NODE && node->content) {
            xmlBufferAdd(buf, node->content, -1);
        }
    }

    // Allocate result string
    char *result;
    if (xmlBufferLength(buf) > 0) {
        result = bgl_strdup((char *)xmlBufferContent(buf));
    } else {
        // No text content, return original
        result = bgl_strdup(input);
    }

    xmlBufferFree(buf);
    xmlFreeDoc(doc);

    return result;
}

int bgl_decode_html_entities(char **inout, bgl_html_option option) {
    if (!inout || !*inout) {
        return -1;
    }

    char *input = *inout;

    // Fast path: check if has HTML entities
    if (!has_html_entities(input)) {
        return 0;  // No change needed
    }

    // Slow path: process the string
    char *new_str = bgl_decode_html_entities_impl(input, option);
    if (!new_str) {
        return -1;
    }

    free(input);
    *inout = new_str;
    return 0;
}

// ============================================================
// Dollar Sign Index Stripping
// ============================================================

/**
 * @brief Check if string contains dollar signs
 */
static bool has_dollar_signs(const char *str) {
    if (!str) return false;
    return strchr(str, '$') != NULL;
}

/**
 * @brief Internal implementation for dollar sign stripping
 * Returns newly allocated string or NULL on failure.
 */
static char *bgl_strip_dollar_indexes_impl(const char *input) {
    if (!input) {
        return NULL;
    }

    // Handle empty string
    if (input[0] == '\0') {
        return bgl_strdup("");
    }

    // Allocate buffer for result (worst case: same size as input)
    size_t input_len = strlen(input);
    char *result = (char *)malloc(input_len + 1);
    if (!result) {
        return NULL;
    }

    size_t result_pos = 0;
    size_t i = 0;
    int strip_count = 0;

    while (i < input_len) {
        // Find first $
        if (input[i] != '$') {
            result[result_pos++] = input[i++];
            continue;
        }

        size_t d0 = i;

        // Find second $
        size_t d1 = d0 + 1;
        while (d1 < input_len && input[d1] != '$') {
            d1++;
        }

        // No closing $ found, copy remaining and exit
        if (d1 >= input_len) {
            // Copy from d0 to end
            while (d0 < input_len) {
                result[result_pos++] = input[d0++];
            }
            break;
        }

        // Case 1: Adjacent $$ - skip all consecutive $
        if (d1 == d0 + 1) {
            // Skip all consecutive $
            i = d1 + 1;
            while (i < input_len && input[i] == '$') {
                i++;
            }
            strip_count++;
            continue;
        }

        // Case 2: Check if content between $ is all digits
        bool all_digits = true;
        for (size_t j = d0 + 1; j < d1; j++) {
            if (!isdigit((unsigned char)input[j])) {
                all_digits = false;
                break;
            }
        }

        // Case 3: $<digits>$ pattern - skip it completely
        if (all_digits) {
            // Skip the entire $<digits>$ pattern
            i = d1 + 1;
            strip_count++;
            continue;
        }

        // Case 4: Non-digit content between $ - keep as-is, move to d1
        // Copy from current position to d1
        for (size_t j = i; j < d1; j++) {
            result[result_pos++] = input[j];
        }
        i = d1;
    }

    result[result_pos] = '\0';

    // Shrink to actual size
    char *shrunk = (char *)realloc(result, result_pos + 1);
    if (shrunk) {
        return shrunk;
    }

    return result;
}

int bgl_strip_dollar_indexes(char **inout) {
    if (!inout || !*inout) {
        return -1;
    }

    char *input = *inout;

    // Fast path: check if has dollar signs
    if (!has_dollar_signs(input)) {
        return 0;  // No change needed
    }

    // Slow path: process the string
    char *new_str = bgl_strip_dollar_indexes_impl(input);
    if (!new_str) {
        return -1;
    }

    free(input);
    *inout = new_str;
    return 0;
}

// ============================================================
// Control Character Removal
// ============================================================

/**
 * @brief Check if character is a BGL control character to be removed
 *
 * Matches pyglossary's regex: [\x00-\x08\x0c\x0e-\x1f]
 * - Removes: 0x00-0x08, 0x0c, 0x0e-0x1f
 * - Preserves: 0x09 (tab), 0x0a (LF), 0x0b (VT), 0x0d (CR)
 */
static bool is_bgl_control_char(char c) {
    unsigned char uc = (unsigned char)c;

    // Control characters to remove
    if (uc <= 0x08) return true;   // \x00-\x08
    if (uc == 0x0c) return true;   // form feed
    if (uc >= 0x0e && uc <= 0x1f) return true;  // \x0e-\x1f

    // Preserve whitespace characters
    // 0x09 = tab, 0x0a = LF, 0x0b = VT, 0x0d = CR
    return false;
}

/**
 * @brief Check if string contains control characters to be removed
 */
static bool has_control_chars(const char *str) {
    if (!str) return false;
    for (const char *p = str; *p; p++) {
        if (is_bgl_control_char(*p)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Internal implementation for control character removal
 * Returns newly allocated string or NULL on failure.
 */
static char *bgl_remove_control_chars_impl(const char *input) {
    if (!input) {
        return NULL;
    }

    // Handle empty string
    if (input[0] == '\0') {
        return bgl_strdup("");
    }

    size_t input_len = strlen(input);

    // Allocate buffer for result
    char *result = (char *)malloc(input_len + 1);
    if (!result) {
        return NULL;
    }

    // Copy only non-control characters
    size_t result_pos = 0;
    for (size_t i = 0; i < input_len; i++) {
        if (!is_bgl_control_char(input[i])) {
            result[result_pos++] = input[i];
        }
    }
    result[result_pos] = '\0';

    // Shrink to actual size
    char *shrunk = (char *)realloc(result, result_pos + 1);
    if (shrunk) {
        return shrunk;
    }

    return result;
}

int bgl_remove_control_chars(char **inout) {
    if (!inout || !*inout) {
        return -1;
    }

    char *str = *inout;

    // Fast path: check if has control characters
    if (!has_control_chars(str)) {
        return 0;
    }

    // In-place removal (output is always <= input)
    char *dst = str;
    for (const char *src = str; *src; src++) {
        if (!is_bgl_control_char(*src)) {
            *dst++ = *src;
        }
    }
    *dst = '\0';
    return 0;
}

// ============================================================
// Newline Removal
// ============================================================

/**
 * @brief Check if string contains newlines
 */
static bool has_newlines(const char *str) {
    if (!str) return false;
    for (const char *p = str; *p; p++) {
        if (*p == '\r' || *p == '\n') {
            return true;
        }
    }
    return false;
}

/**
 * @brief Internal implementation for newline removal
 * Returns newly allocated string or NULL on failure.
 */
static char *bgl_remove_newlines_impl(const char *input) {
    if (!input) {
        return NULL;
    }

    // Handle empty string
    if (input[0] == '\0') {
        return bgl_strdup("");
    }

    size_t input_len = strlen(input);

    // Allocate buffer for result (worst case: same size as input)
    char *result = (char *)malloc(input_len + 1);
    if (!result) {
        return NULL;
    }

    size_t result_pos = 0;
    bool last_was_newline = false;

    for (size_t i = 0; i < input_len; i++) {
        char c = input[i];

        // Check if current char is newline (\r or \n)
        if (c == '\r' || c == '\n') {
            // Only add space if we haven't just added one
            if (!last_was_newline) {
                result[result_pos++] = ' ';
                last_was_newline = true;
            }
            // Skip the newline character itself
            // (for \r\n sequences, this skips both after adding one space)
        } else {
            result[result_pos++] = c;
            last_was_newline = false;
        }
    }

    result[result_pos] = '\0';

    // Shrink to actual size
    char *shrunk = (char *)realloc(result, result_pos + 1);
    if (shrunk) {
        return shrunk;
    }

    return result;
}

int bgl_remove_newlines(char **inout) {
    if (!inout || !*inout) {
        return -1;
    }

    char *str = *inout;

    // Fast path
    if (!has_newlines(str)) {
        return 0;
    }

    // In-place: replace newlines with spaces, collapse consecutive
    char *dst = str;
    bool last_was_newline = false;

    for (const char *src = str; *src; src++) {
        if (*src == '\r' || *src == '\n') {
            if (!last_was_newline) {
                *dst++ = ' ';
                last_was_newline = true;
            }
        } else {
            *dst++ = *src;
            last_was_newline = false;
        }
    }
    *dst = '\0';
    return 0;
}

// ============================================================
// Slash Alt Key Pattern Stripping
// ============================================================

/**
 * @brief Check if string contains slash pattern that needs to be stripped
 * Looks for "/" at start or after whitespace followed by word character
 */
static bool has_slash_alt_pattern(const char *str) {
    if (!str || str[0] == '\0') return false;

    // Check if starts with "/word"
    if (str[0] == '/' && isalnum((unsigned char)str[1])) {
        return true;
    }

    // Check for whitespace + "/word"
    for (const char *p = str; *p; p++) {
        if (isspace((unsigned char)*p) && p[1] == '/' && isalnum((unsigned char)p[2])) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Internal implementation for slash alt key pattern stripping
 * Returns newly allocated string or NULL on failure.
 */
static char *bgl_strip_slash_alt_key_impl(const char *input) {
    if (!input) {
        return NULL;
    }

    // Handle empty string
    if (input[0] == '\0') {
        return bgl_strdup("");
    }

    size_t input_len = strlen(input);

    // Allocate buffer for result (worst case: same size as input)
    char *result = (char *)malloc(input_len + 1);
    if (!result) {
        return NULL;
    }

    size_t result_pos = 0;
    size_t i = 0;

    while (i < input_len) {
        // Check for "/" that's at start or after whitespace
        if (input[i] == '/') {
            // Check if this is at the start of the string
            bool at_start = (i == 0);
            // Check if this is after whitespace
            bool after_whitespace = (i > 0 && isspace((unsigned char)input[i - 1]));

            if (at_start || after_whitespace) {
                // Check if followed by word character (per regex pattern)
                if (i + 1 < input_len && isalnum((unsigned char)input[i + 1])) {
                    // Skip the "/" (don't copy it)
                    i++;
                    continue;
                }
            }
        }

        // Copy current character
        result[result_pos++] = input[i++];
    }

    result[result_pos] = '\0';

    // Shrink to actual size
    char *shrunk = (char *)realloc(result, result_pos + 1);
    if (shrunk) {
        return shrunk;
    }

    return result;
}

int bgl_strip_slash_alt_key(char **inout) {
    if (!inout || !*inout) {
        return -1;
    }

    char *str = *inout;

    // Fast path
    if (!has_slash_alt_pattern(str)) {
        return 0;
    }

    // In-place removal of "/" at start or after whitespace before word char
    char *dst = str;
    for (const char *src = str; *src; src++) {
        if (*src == '/') {
            bool at_start = (src == str);
            bool after_ws = (src > str && isspace((unsigned char)*(src - 1)));
            if ((at_start || after_ws) && isalnum((unsigned char)*(src + 1))) {
                continue;  // skip this '/'
            }
        }
        *dst++ = *src;
    }
    *dst = '\0';
    return 0;
}

// ============================================================
// Image Link Fixing
// ============================================================

/**
 * @brief Check if string contains BGL control characters for image links
 */
static bool has_img_link_control_chars(const char *str) {
    if (!str) return false;
    return (strchr(str, 0x1e) != NULL || strchr(str, 0x1f) != NULL);
}

/**
 * @brief Internal implementation for fixing image links
 * Returns newly allocated string or NULL on failure.
 */
static char *bgl_fix_img_links_impl(const char *input) {
    if (!input) {
        return NULL;
    }

    // Handle empty string
    if (input[0] == '\0') {
        return bgl_strdup("");
    }

    size_t input_len = strlen(input);

    // Allocate buffer for result (worst case: same size as input)
    char *result = (char *)malloc(input_len + 1);
    if (!result) {
        return NULL;
    }

    // Copy all characters except \x1e and \x1f
    size_t result_pos = 0;
    for (size_t i = 0; i < input_len; i++) {
        unsigned char c = (unsigned char)input[i];
        if (c != 0x1e && c != 0x1f) {
            result[result_pos++] = input[i];
        }
    }

    result[result_pos] = '\0';

    // Shrink to actual size
    char *shrunk = (char *)realloc(result, result_pos + 1);
    if (shrunk) {
        return shrunk;
    }

    return result;
}

int bgl_fix_img_links(char **inout) {
    if (!inout || !*inout) {
        return -1;
    }

    char *str = *inout;

    // Fast path
    if (!has_img_link_control_chars(str)) {
        return 0;
    }

    // In-place removal of \x1e and \x1f
    char *dst = str;
    for (const char *src = str; *src; src++) {
        unsigned char c = (unsigned char)*src;
        if (c != 0x1e && c != 0x1f) {
            *dst++ = *src;
        }
    }
    *dst = '\0';
    return 0;
}

// ============================================================
// Newline Normalization
// ============================================================

/**
 * @brief Internal implementation for normalizing newlines
 * Returns newly allocated string or NULL on failure.
 */
static char *bgl_normalize_newlines_impl(const char *input) {
    if (!input) {
        return NULL;
    }

    // Handle empty string
    if (input[0] == '\0') {
        return bgl_strdup("");
    }

    size_t input_len = strlen(input);

    // Allocate buffer for result (worst case: same size as input)
    char *result = (char *)malloc(input_len + 1);
    if (!result) {
        return NULL;
    }

    size_t result_pos = 0;
    bool last_was_newline = false;

    for (size_t i = 0; i < input_len; i++) {
        char c = input[i];

        // Check if current char is newline (\r or \n)
        if (c == '\r' || c == '\n') {
            // Only add \n if we haven't just added one
            if (!last_was_newline) {
                result[result_pos++] = '\n';
                last_was_newline = true;
            }
            // Skip the newline character itself
            // (for \r\n sequences, this skips both after adding one \n)
        } else {
            result[result_pos++] = c;
            last_was_newline = false;
        }
    }

    result[result_pos] = '\0';

    // Shrink to actual size
    char *shrunk = (char *)realloc(result, result_pos + 1);
    if (shrunk) {
        return shrunk;
    }

    return result;
}

int bgl_normalize_newlines(char **inout) {
    if (!inout || !*inout) {
        return -1;
    }

    char *str = *inout;

    // Fast path
    if (!has_newlines(str)) {
        return 0;
    }

    // In-place: collapse all newline sequences to single \n
    char *dst = str;
    bool last_was_newline = false;

    for (const char *src = str; *src; src++) {
        if (*src == '\r' || *src == '\n') {
            if (!last_was_newline) {
                *dst++ = '\n';
                last_was_newline = true;
            }
        } else {
            *dst++ = *src;
            last_was_newline = false;
        }
    }
    *dst = '\0';
    return 0;
}

// ============================================================
// String Strip (like Python's str.strip())
// ============================================================

/**
 * @brief Check if string has leading or trailing whitespace
 */
static bool has_leading_trailing_whitespace(const char *str) {
    if (!str || str[0] == '\0') return false;

    // Check for leading whitespace
    if (isspace((unsigned char)str[0])) {
        return true;
    }

    size_t len = strlen(str);
    if (len == 0) return false;

    // Check for trailing whitespace
    if (isspace((unsigned char)str[len - 1])) {
        return true;
    }

    return false;
}

/**
 * @brief Internal implementation for stripping whitespace
 * Returns newly allocated string or NULL on failure.
 */
static char *bgl_strip_impl(const char *input) {
    if (!input) {
        return NULL;
    }

    // Handle empty string
    if (input[0] == '\0') {
        return bgl_strdup("");
    }

    size_t input_len = strlen(input);

    // Find first non-whitespace character
    size_t start = 0;
    while (start < input_len && isspace((unsigned char)input[start])) {
        start++;
    }

    // All whitespace — return empty string
    if (start >= input_len) {
        return bgl_strdup("");
    }

    // Find last non-whitespace character
    size_t end = input_len - 1;
    while (end > start && isspace((unsigned char)input[end])) {
        end--;
    }

    // Calculate length of stripped string
    size_t stripped_len = end - start + 1;

    // Allocate and copy
    char *result = (char *)malloc(stripped_len + 1);
    if (!result) {
        return NULL;
    }

    memcpy(result, input + start, stripped_len);
    result[stripped_len] = '\0';

    return result;
}

int bgl_strip(char **inout) {
    if (!inout || !*inout) {
        return -1;
    }

    char *input = *inout;

    // Fast path: check if has leading/trailing whitespace
    if (!has_leading_trailing_whitespace(input)) {
        return 0;  // No change needed
    }

    // Slow path: process the string
    char *new_str = bgl_strip_impl(input);
    if (!new_str) {
        return -1;
    }

    free(input);
    *inout = new_str;
    return 0;
}

// ============================================================
// HTML Tag Stripping
// ============================================================

/**
 * @brief Check if string contains HTML tags
 */
static bool has_html_tags(const char *str) {
    if (!str) return false;
    return strchr(str, '<') != NULL;
}

/**
 * @brief Internal implementation for stripping HTML tags
 * Returns newly allocated string or NULL on failure.
 */
static char *bgl_strip_html_tags_impl(const char *input) {
    if (!input) {
        return NULL;
    }

    // Handle empty string
    if (input[0] == '\0') {
        return bgl_strdup("");
    }

    size_t input_len = strlen(input);

    // Allocate buffer for result (worst case: same size as input)
    char *result = (char *)malloc(input_len + 1);
    if (!result) {
        return NULL;
    }

    size_t result_pos = 0;
    bool in_tag = false;

    for (size_t i = 0; i < input_len; i++) {
        char c = input[i];

        if (c == '<') {
            // Start of HTML tag
            in_tag = true;
        } else if (c == '>') {
            // End of HTML tag
            in_tag = false;
            // Replace tag with space
            result[result_pos++] = ' ';
        } else if (!in_tag) {
            // Copy character if not inside a tag
            result[result_pos++] = c;
        }
        // If in_tag is true, skip the character (don't copy)
    }

    result[result_pos] = '\0';

    // Shrink to actual size
    char *shrunk = (char *)realloc(result, result_pos + 1);
    if (shrunk) {
        return shrunk;
    }

    return result;
}

int bgl_strip_html_tags(char **inout) {
    if (!inout || !*inout) {
        return -1;
    }

    char *input = *inout;

    // Fast path: check if has HTML tags
    if (!has_html_tags(input)) {
        return 0;  // No change needed
    }

    // Slow path: process the string
    char *new_str = bgl_strip_html_tags_impl(input);
    if (!new_str) {
        return -1;
    }

    free(input);
    *inout = new_str;
    return 0;
}
