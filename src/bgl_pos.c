//
//  bgl_pos.c
//  libud
//
//  Created by kejinlu on 2026-02-02
//

#include "bgl_pos.h"
#include <stdlib.h>
#include <string.h>

// ============================================================
// Constants
// ============================================================

#define POS_COUNT 24  // 0x47 - 0x30 + 1

// ============================================================
// Internal Data Structures
// ============================================================

typedef struct {
    const char *name;     /**< Full name */
    const char *abbr;     /**< Abbreviation */
} bgl_pos_entry;

// ============================================================
// Part-of-Speech Table
// ============================================================
// Reference: pyglossary bgl_pos.py and GoldenDict bgl_babylon.hh
// Code range: 0x30-0x47 (24 consecutive values)
// Uses code - 0x30 as array index for O(1) lookup

static const bgl_pos_entry part_of_speeches[POS_COUNT] = {
    /* 0x30 */ {"noun",                   "n."},
    /* 0x31 */ {"adjective",              "adj."},
    /* 0x32 */ {"verb",                   "v."},
    /* 0x33 */ {"adverb",                 "adv."},
    /* 0x34 */ {"interjection",           "interj."},
    /* 0x35 */ {"pronoun",                "pron."},
    /* 0x36 */ {"preposition",            "prep."},
    /* 0x37 */ {"conjunction",            "conj."},
    /* 0x38 */ {"suffix",                 "suff."},
    /* 0x39 */ {"prefix",                 "pref."},
    /* 0x3A */ {"article",                "art."},
    /* 0x3B */ {"unknown",                ""},      // Seen in some dictionaries, meaning unclear
    /* 0x3C */ {"abbreviation",           "abbr."}, // Common in Hebrew dictionaries
    /* 0x3D */ {"masculine noun and adjective", ""},
    /* 0x3E */ {"feminine noun and adjective", ""},
    /* 0x3F */ {"masculine and feminine noun and adjective", ""},
    /* 0x40 */ {"feminine noun",          "f. n."},
    /* 0x41 */ {"masculine and feminine noun", ""},
    /* 0x42 */ {"masculine noun",         "m. n."},
    /* 0x43 */ {"numeral",                "num."},
    /* 0x44 */ {"participle",             "part."},
    /* 0x45 */ {"unknown",                ""},      // Undefined
    /* 0x46 */ {"unknown",                ""},      // Undefined
    /* 0x47 */ {"unknown",                ""},      // Undefined
};

// ============================================================
// Function Implementations
// ============================================================

const char *bgl_pos_name_by_code(int code) {
    if (!bgl_pos_is_valid(code)) {
        return NULL;
    }
    return part_of_speeches[code - 0x30].name;
}

const char *bgl_pos_abbr_by_code(int code) {
    if (!bgl_pos_is_valid(code)) {
        return NULL;
    }
    return part_of_speeches[code - 0x30].abbr;
}

bool bgl_pos_is_valid(int code) {
    return code >= 0x30 && code <= 0x47;
}
