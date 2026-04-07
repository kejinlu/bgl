//
//  bgl_util.h
//  libud
//
//  Created by kejinlu on 2026-02-02
//

#ifndef bgl_util_h
#define bgl_util_h

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Byte Order Helpers (inline for zero overhead)
// ============================================================

/**
 * @brief Read big-endian 16-bit unsigned integer
 */
static inline uint16_t bgl_read_uint16_be(const uint8_t *data) {
    return (uint16_t)((data[0] << 8) | data[1]);
}

/**
 * @brief Read big-endian 32-bit unsigned integer
 */
static inline uint32_t bgl_read_uint32_be(const uint8_t *data) {
    return ((uint32_t)data[0] << 24) |
           ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8) |
           ((uint32_t)data[3]);
}

// ============================================================
// UTF-8 Encoding
// ============================================================

/**
 * @brief Encode a Unicode code point into UTF-8
 * @param code Unicode code point
 * @param buf Output buffer (must be at least 4 bytes)
 * @return Number of bytes written (1-4), or 0 if code point is invalid
 */
static inline size_t bgl_codepoint_to_utf8(uint32_t code, char *buf) {
    if (code < 0x80) {
        buf[0] = (char)code;
        return 1;
    } else if (code < 0x800) {
        buf[0] = (char)(0xC0 | (code >> 6));
        buf[1] = (char)(0x80 | (code & 0x3F));
        return 2;
    } else if (code < 0x10000) {
        buf[0] = (char)(0xE0 | (code >> 12));
        buf[1] = (char)(0x80 | ((code >> 6) & 0x3F));
        buf[2] = (char)(0x80 | (code & 0x3F));
        return 3;
    } else if (code < 0x110000) {
        buf[0] = (char)(0xF0 | (code >> 18));
        buf[1] = (char)(0x80 | ((code >> 12) & 0x3F));
        buf[2] = (char)(0x80 | ((code >> 6) & 0x3F));
        buf[3] = (char)(0x80 | (code & 0x3F));
        return 4;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* bgl_util_h */
