//
//  bgl_language.c
//  libud
//
//  Created by kejinlu on 2026-02-02
//

#include "bgl_language.h"
#include <string.h>

// ============================================================
// Language Table Definition
// ============================================================

static const bgl_language languages[] = {
    {"English", "en", "CP1252", 0x00},
    {"French", "fr", "CP1252", 0x01},
    {"Italian", "it", "CP1252", 0x02},
    {"Spanish", "es", "CP1252", 0x03},
    {"Dutch", "nl", "CP1252", 0x04},
    {"Portuguese", "pt", "CP1252", 0x05},
    {"German", "de", "CP1252", 0x06},
    {"Russian", "ru", "CP1251", 0x07},
    {"Japanese", "ja", "CP932", 0x08},
    {"Chinese", "zh-Hant", "CP950", 0x09},
    {"Chinese", "zh-Hans", "CP936", 0x0a},
    {"Greek", "el", "CP1253", 0x0b},
    {"Korean", "ko", "CP949", 0x0c},
    {"Turkish", "tr", "CP1254", 0x0d},
    {"Hebrew", "he", "CP1255", 0x0e},
    {"Arabic", "ar", "CP1256", 0x0f},
    {"Thai", "th", "CP874", 0x10},
    {"Other", "und", "CP1252", 0x11},
    {"Chinese", "zh-Hans", "CP936", 0x12},
    {"Chinese", "zh-Hant", "CP950", 0x13},
    {"Other Eastern-European languages", "und", "CP1250", 0x14},
    {"Other Western-European languages", "und", "CP1252", 0x15},
    {"Other Russian languages", "ru", "CP1251", 0x16},
    {"Other Japanese languages", "ja", "CP932", 0x17},
    {"Other Baltic languages", "und", "CP1257", 0x18},
    {"Other Greek languages", "el", "CP1253", 0x19},
    {"Other Korean dialects", "ko", "CP949", 0x1a},
    {"Other Turkish dialects", "tr", "CP1254", 0x1b},
    {"Other Thai dialects", "th", "CP874", 0x1c},
    {"Polish", "pl", "CP1250", 0x1d},
    {"Hungarian", "hu", "CP1250", 0x1e},
    {"Czech", "cs", "CP1250", 0x1f},
    {"Lithuanian", "lt", "CP1257", 0x20},
    {"Latvian", "lv", "CP1257", 0x21},
    {"Catalan", "ca", "CP1252", 0x22},
    {"Croatian", "hr", "CP1250", 0x23},
    {"Serbian", "sr-Latn", "CP1250", 0x24},
    {"Slovak", "sk", "CP1250", 0x25},
    {"Albanian", "sq", "CP1252", 0x26},
    {"Urdu", "ur", "CP1256", 0x27},
    {"Slovenian", "sl", "CP1250", 0x28},
    {"Estonian", "et", "CP1252", 0x29},
    {"Bulgarian", "bg", "CP1250", 0x2a},
    {"Danish", "da", "CP1252", 0x2b},
    {"Finnish", "fi", "CP1252", 0x2c},
    {"Icelandic", "is", "CP1252", 0x2d},
    {"Norwegian", "no", "CP1252", 0x2e},
    {"Romanian", "ro", "CP1252", 0x2f},
    {"Swedish", "sv", "CP1252", 0x30},
    {"Ukrainian", "uk", "CP1251", 0x31},
    {"Belarusian", "be", "CP1251", 0x32},
    {"Persian", "fa", "CP1256", 0x33},
    {"Basque", "eu", "CP1252", 0x34},
    {"Macedonian", "mk", "CP1250", 0x35},
    {"Afrikaans", "af", "CP1252", 0x36},
    {"Faroese", "fo", "CP1252", 0x37},
    {"Latin", "la", "CP1252", 0x38},
    {"Esperanto", "eo", "CP1254", 0x39},
    {"Tamazight", "tzm", "CP1252", 0x3a},
    {"Armenian", "hy", "CP1252", 0x3b},
    {"Hindi", "hi", "CP1252", 0x3c},
    {"Somali", "so", "CP1252", 0x3d},
};

#define LANGUAGES_COUNT (sizeof(languages) / sizeof(languages[0]))

// ============================================================
// Function Implementations
// ============================================================

const bgl_language *bgl_language_by_code(int code) {
    if (code < 0 || code >= (int)LANGUAGES_COUNT) {
        return NULL;
    }
    // Codes 0x00-0x3d are stored contiguously — direct index O(1)
    return &languages[code];
}

const bgl_language *bgl_language_by_name(const char *name) {
    if (!name) {
        return NULL;
    }

    for (size_t i = 0; i < LANGUAGES_COUNT; i++) {
        if (strcmp(languages[i].name, name) == 0) {
            return &languages[i];
        }
    }

    return NULL;
}

const bgl_language *bgl_language_by_bcp47(const char *bcp47) {
    if (!bcp47) {
        return NULL;
    }

    for (size_t i = 0; i < LANGUAGES_COUNT; i++) {
        if (strcmp(languages[i].bcp47, bcp47) == 0) {
            return &languages[i];
        }
    }

    return NULL;
}

int bgl_language_count(void) {
    return (int)LANGUAGES_COUNT;
}

const bgl_language *bgl_language_at(int index) {
    if (index < 0 || index >= (int)LANGUAGES_COUNT) {
        return NULL;
    }
    return &languages[index];
}

// ============================================================
// Charset Mapping (merged from bgl_charset)
// ============================================================
// Reference: BGL charset codes (0x41-0x4e, 14 consecutive values)
// Uses code - 0x41 as array index for O(1) lookup

#define CHARSET_COUNT 14 // 0x4e - 0x41 + 1

static const char *const charsets[CHARSET_COUNT] = {
    /* 0x41 */ "CP1252", // Default
    /* 0x42 */ "CP1252", // Latin
    /* 0x43 */ "CP1250", // Eastern European
    /* 0x44 */ "CP1251", // Cyrillic
    /* 0x45 */ "CP932",  // Japanese
    /* 0x46 */ "CP950",  // Traditional Chinese
    /* 0x47 */ "CP936",  // Simplified Chinese
    /* 0x48 */ "CP1257", // Baltic
    /* 0x49 */ "CP1253", // Greek
    /* 0x4a */ "CP949",  // Korean
    /* 0x4b */ "CP1254", // Turkish
    /* 0x4c */ "CP1255", // Hebrew
    /* 0x4d */ "CP1256", // Arabic
    /* 0x4e */ "CP874",  // Thai
};

const char *bgl_charset_by_code(int code) {
    if (code < 0x41 || code > 0x4e) {
        return NULL;
    }
    return charsets[code - 0x41];
}

const char *bgl_charset_default(void) {
    return "CP1252";
}
