//
//  bgl_info.c
//  libud
//
//  Created by kejinlu on 2026-02-02
//

#include "bgl_info.h"
#include "bgl_language.h"
#include "bgl_util.h"
#include "bgl_port.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iconv.h>

// ============================================================
// Internal Helper Functions
// ============================================================

/**
 * @brief Convert date to Julian day
 */
static int date_to_jd(int year, int month, int day) {
    // Simplified Julian day calculation
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    return day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
}

/**
 * @brief Convert Julian day to date
 */
static void jd_to_date(int jd, int *year, int *month, int *day) {
    int a = jd + 32044;
    int b = (4 * a + 3) / 146097;
    int c = a - (146097 * b) / 4;
    int d = (4 * c + 3) / 1461;
    int e = c - (1461 * d) / 4;
    int m = (5 * e + 2) / 153;
    *day = e - (153 * m + 2) / 5 + 1;
    *month = m + 3 - 12 * (m / 10);
    *year = 100 * b + d - 4800 + m / 10;
}

// ============================================================
// Function Implementations
// ============================================================

static int bgl_parse_flags_field(const uint8_t *data, bgl_flags *flags_out) {
    if (!data || !flags_out) {
        return -1;
    }

    uint32_t flags = bgl_read_uint32_be(data);

    flags_out->utf8_encoding = (flags & 0x8000) != 0;
    flags_out->spelling_alternatives = (flags & 0x10000) == 0;
    flags_out->case_sensitive = (flags & 0x1000) != 0;

    return 0;
}

static int bgl_parse_timestamp_field(const uint8_t *data, bgl_timestamp *ts_out) {
    if (!data || !ts_out) {
        return -1;
    }

    uint32_t value = bgl_read_uint32_be(data);

    // Julian day for 1970-01-01
    const int jd1970 = date_to_jd(1970, 1, 1);

    // Separate days and minutes
    int djd, hm;
    djd = value / (24 * 60);
    hm = value % (24 * 60);

    // Convert to date
    jd_to_date(jd1970 + djd, &ts_out->year, &ts_out->month, &ts_out->day);

    // Convert to hours and minutes
    ts_out->hour = hm / 60;
    ts_out->minute = hm % 60;

    return 0;
}

static char *bgl_format_timestamp(const bgl_timestamp *ts, char *buffer, size_t buffer_size) {
    if (!ts || !buffer || buffer_size < 20) {
        return NULL;
    }

    snprintf(buffer, buffer_size, "%04d/%02d/%02d, %02d:%02d",
             ts->year, ts->month, ts->day,
             ts->hour, ts->minute);

    return buffer;
}

int bgl_parse_info_field(const uint8_t *data, size_t data_size, bgl_info *info) {
    if (!data || data_size < 2 || !info) {
        return -1;
    }

    // Read 2-byte big-endian code
    uint16_t code = (data[0] << 8) | data[1];
    const uint8_t *b_value = data + 2;
    size_t b_value_len = data_size - 2;

    // Skip all-zero values
    bool all_zero = true;
    for (size_t i = 0; i < b_value_len; i++) {
        if (b_value[i] != 0) {
            all_zero = false;
            break;
        }
    }
    if (all_zero) {
        return 0;
    }

    // Parse and directly assign to info structure
    switch (code) {
        case BGL_INFO_TITLE:
            if (b_value_len > 0) {
                free(info->title);
                info->title = (char *)malloc(b_value_len + 1);
                if (info->title) {
                    memcpy(info->title, b_value, b_value_len);
                    info->title[b_value_len] = '\0';
                }
            }
            break;

        case BGL_INFO_AUTHOR:
            if (b_value_len > 0) {
                free(info->author);
                info->author = (char *)malloc(b_value_len + 1);
                if (info->author) {
                    memcpy(info->author, b_value, b_value_len);
                    info->author[b_value_len] = '\0';
                }
            }
            break;

        case BGL_INFO_EMAIL:
            if (b_value_len > 0) {
                free(info->email);
                info->email = (char *)malloc(b_value_len + 1);
                if (info->email) {
                    memcpy(info->email, b_value, b_value_len);
                    info->email[b_value_len] = '\0';
                }
            }
            break;

        case BGL_INFO_COPYRIGHT:
            if (b_value_len > 0) {
                free(info->copyright);
                info->copyright = (char *)malloc(b_value_len + 1);
                if (info->copyright) {
                    memcpy(info->copyright, b_value, b_value_len);
                    info->copyright[b_value_len] = '\0';
                }
            }
            break;

        case BGL_INFO_DESCRIPTION:
            if (b_value_len > 0) {
                free(info->description);
                info->description = (char *)malloc(b_value_len + 1);
                if (info->description) {
                    memcpy(info->description, b_value, b_value_len);
                    info->description[b_value_len] = '\0';
                }
            }
            break;

        case BGL_INFO_SOURCE_LANG:
            if (b_value_len >= 4) {
                uint32_t lang_code = bgl_read_uint32_be(b_value);
                const bgl_language *lang = bgl_language_by_code((int)lang_code);
                if (lang && lang->name) {
                    free(info->source_lang);
                    info->source_lang = bgl_strdup(lang->name);
                }
            }
            break;

        case BGL_INFO_TARGET_LANG:
            if (b_value_len >= 4) {
                uint32_t lang_code = bgl_read_uint32_be(b_value);
                const bgl_language *lang = bgl_language_by_code((int)lang_code);
                if (lang && lang->name) {
                    free(info->target_lang);
                    info->target_lang = bgl_strdup(lang->name);
                }
            }
            break;

        case BGL_INFO_SOURCE_CHARSET:
            if (b_value_len >= 1) {
                int charset_code = b_value[0];
                const char *encoding = bgl_charset_by_code(charset_code);
                if (encoding) {
                    info->source_charset = (char *)encoding;  // Static string, do not free
                }
            }
            break;

        case BGL_INFO_TARGET_CHARSET:
            if (b_value_len >= 1) {
                int charset_code = b_value[0];
                const char *encoding = bgl_charset_by_code(charset_code);
                if (encoding) {
                    info->target_charset = (char *)encoding;  // Static string, do not free
                }
            }
            break;

        case BGL_INFO_FLAGS:
            if (b_value_len >= 4) {
                bgl_flags flags;
                if (bgl_parse_flags_field(b_value, &flags) == 0) {
                    info->utf8_mode = flags.utf8_encoding;
                }
            }
            break;

        case BGL_INFO_NUM_ENTRIES:
            if (b_value_len >= 4) {
                uint32_t num_entries = bgl_read_uint32_be(b_value);
                info->entry_count = (int)num_entries;
            }
            break;

        // Other types are not stored in bgl_info structure
        // They can be handled separately if needed
        default:
            break;
    }

    return 0;
}

// ============================================================
// Encoding Conversion
// ============================================================

/**
 * @brief Internal helper: convert a string from encoding to UTF-8
 */
static char *convert_to_utf8(const char *input, const char *encoding) {
    if (!input || !encoding) {
        return NULL;
    }

    // Check if already UTF-8
    if (bgl_strcasecmp(encoding, "UTF-8") == 0) {
        return bgl_strdup(input);
    }

    // Use iconv to convert
    iconv_t cd = iconv_open("UTF-8", encoding);
    if (cd == (iconv_t)-1) {
        return NULL;
    }

    size_t inbytes_left = strlen(input);
    size_t outbuf_size = inbytes_left * 4;
    char *outbuf = (char *)malloc(outbuf_size);
    if (!outbuf) {
        iconv_close(cd);
        return NULL;
    }

    char *inptr = (char *)input;
    char *outptr = outbuf;

    size_t result = iconv(cd, &inptr, &inbytes_left, &outptr, &outbuf_size);
    iconv_close(cd);

    if (result == (size_t)-1) {
        free(outbuf);
        return NULL;
    }

    *outptr = '\0';
    return outbuf;
}

/**
 * @brief Internal helper: convert and replace a string field
 */
static void convert_field(char **field, const char *encoding) {
    if (!field || !*field || !encoding) {
        return;
    }

    char *converted = convert_to_utf8(*field, encoding);
    if (converted) {
        free(*field);
        *field = converted;
    }
}

int bgl_convert_info_to_utf8(bgl_info *info, const char *encoding) {
    if (!info || !encoding) {
        return -1;
    }

    // Early return if already UTF-8
    if (bgl_strcasecmp(encoding, "UTF-8") == 0) {
        return 0;
    }

    // Convert all text fields
    convert_field(&info->title, encoding);
    convert_field(&info->author, encoding);
    convert_field(&info->email, encoding);
    convert_field(&info->copyright, encoding);
    convert_field(&info->description, encoding);

    return 0;
}

void bgl_free_info(bgl_info *info) {
    if (!info) {
        return;
    }

    free(info->title);        info->title = NULL;
    free(info->author);       info->author = NULL;
    free(info->email);        info->email = NULL;
    free(info->copyright);    info->copyright = NULL;
    free(info->description);  info->description = NULL;
    free(info->source_lang);  info->source_lang = NULL;
    free(info->target_lang);  info->target_lang = NULL;

    // source_charset and target_charset point to static strings, do not free
    info->source_charset = NULL;
    info->target_charset = NULL;

    info->utf8_mode = false;
    info->entry_count = 0;
}
