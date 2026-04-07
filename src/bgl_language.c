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
    {"English", "", "CP1252", 0x00},
    {"French", "", "CP1252", 0x01},
    {"Italian", "", "CP1252", 0x02},
    {"Spanish", "", "CP1252", 0x03},
    {"Dutch", "", "CP1252", 0x04},
    {"Portuguese", "", "CP1252", 0x05},
    {"German", "", "CP1252", 0x06},
    {"Russian", "", "CP1251", 0x07},
    {"Japanese", "", "CP932", 0x08},
    {"Chinese", "Traditional Chinese", "CP950", 0x09},
    {"Chinese", "Simplified Chinese", "CP936", 0x0a},
    {"Greek", "", "CP1253", 0x0b},
    {"Korean", "", "CP949", 0x0c},
    {"Turkish", "", "CP1254", 0x0d},
    {"Hebrew", "", "CP1255", 0x0e},
    {"Arabic", "", "CP1256", 0x0f},
    {"Thai", "", "CP874", 0x10},
    {"Other", "", "CP1252", 0x11},
    {"Chinese", "Other Simplified Chinese dialects", "CP936", 0x12},
    {"Chinese", "Other Traditional Chinese dialects", "CP950", 0x13},
    {"Other Eastern-European languages", "", "CP1250", 0x14},
    {"Other Western-European languages", "", "CP1252", 0x15},
    {"Other Russian languages", "", "CP1251", 0x16},
    {"Other Japanese languages", "", "CP932", 0x17},
    {"Other Baltic languages", "", "CP1257", 0x18},
    {"Other Greek languages", "", "CP1253", 0x19},
    {"Other Korean dialects", "", "CP949", 0x1a},
    {"Other Turkish dialects", "", "CP1254", 0x1b},
    {"Other Thai dialects", "", "CP874", 0x1c},
    {"Polish", "", "CP1250", 0x1d},
    {"Hungarian", "", "CP1250", 0x1e},
    {"Czech", "", "CP1250", 0x1f},
    {"Lithuanian", "", "CP1257", 0x20},
    {"Latvian", "", "CP1257", 0x21},
    {"Catalan", "", "CP1252", 0x22},
    {"Croatian", "", "CP1250", 0x23},
    {"Serbian", "", "CP1250", 0x24},
    {"Slovak", "", "CP1250", 0x25},
    {"Albanian", "", "CP1252", 0x26},
    {"Urdu", "", "CP1256", 0x27},
    {"Slovenian", "", "CP1250", 0x28},
    {"Estonian", "", "CP1252", 0x29},
    {"Bulgarian", "", "CP1250", 0x2a},
    {"Danish", "", "CP1252", 0x2b},
    {"Finnish", "", "CP1252", 0x2c},
    {"Icelandic", "", "CP1252", 0x2d},
    {"Norwegian", "", "CP1252", 0x2e},
    {"Romanian", "", "CP1252", 0x2f},
    {"Swedish", "", "CP1252", 0x30},
    {"Ukrainian", "", "CP1251", 0x31},
    {"Belarusian", "", "CP1251", 0x32},
    {"Persian", "Farsi", "CP1256", 0x33},
    {"Basque", "", "CP1252", 0x34},
    {"Macedonian", "", "CP1250", 0x35},
    {"Afrikaans", "", "CP1252", 0x36},
    {"Faroese", "", "CP1252", 0x37},
    {"Latin", "", "CP1252", 0x38},
    {"Esperanto", "", "CP1254", 0x39},
    {"Tamazight", "Standard Moroccan Tamazight", "CP1252", 0x3a},
    {"Armenian", "", "CP1252", 0x3b},
    {"Hindi", "", "CP1252", 0x3c},
    {"Somali", "", "CP1252", 0x3d},
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

#define CHARSET_COUNT 14  // 0x4e - 0x41 + 1

static const char *const charsets[CHARSET_COUNT] = {
    /* 0x41 */ "CP1252",  // Default
    /* 0x42 */ "CP1252",  // Latin
    /* 0x43 */ "CP1250",  // Eastern European
    /* 0x44 */ "CP1251",  // Cyrillic
    /* 0x45 */ "CP932",   // Japanese
    /* 0x46 */ "CP950",   // Traditional Chinese
    /* 0x47 */ "CP936",   // Simplified Chinese
    /* 0x48 */ "CP1257",  // Baltic
    /* 0x49 */ "CP1253",  // Greek
    /* 0x4a */ "CP949",   // Korean
    /* 0x4b */ "CP1254",  // Turkish
    /* 0x4c */ "CP1255",  // Hebrew
    /* 0x4d */ "CP1256",  // Arabic
    /* 0x4e */ "CP874",   // Thai
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
