# BGL Binary File Format

## Overview

BGL is the proprietary binary format used by Babylon dictionary software. The file
consists of a small plaintext header followed by a gzip-compressed stream containing
all dictionary data (metadata, entries, and embedded resources).

All multi-byte integers in the format are **big-endian**.

## File Structure

```
+--------------------------------------------------+
|  File Header (6 bytes)                           |
+--------------------------------------------------+
|  Unknown data (gzip_offset - 6 bytes)            |  <-- not parsed
+--------------------------------------------------+
|  Gzip-compressed stream                          |
|  +--------------------------------------------+  |
|  | Type 0 blocks  - extended metadata         |  |
|  | Type 3 blocks  - info fields               |  |
|  | Type 6 block   - entries section start     |  |
|  | Type 1/7/10/11/13 - dictionary entries     |  |
|  | Type 2 blocks  - embedded resources        |  |
|  | Type 4 block   - EOF marker                |  |
|  +--------------------------------------------+  |
+--------------------------------------------------+
```

The file header contains a `gzip_offset` field that points to where the gzip
stream begins. The bytes between the 6-byte header and `gzip_offset` are
currently not parsed by this implementation.

Info blocks (Type 3) may be scattered throughout the stream, including after
entries and resources. A full scan is required to collect all metadata.

---

## 1. File Header

Fixed 6 bytes at the beginning of the file.

| Offset | Size | Field         | Description                        |
|--------|------|---------------|------------------------------------|
| 0      | 4    | signature     | File signature (big-endian uint32) |
| 4      | 2    | gzip_offset   | Offset to gzip data (big-endian uint16) |

### Signature

Two valid values:

| Value        | Description    |
|--------------|----------------|
| `0x12340001` | BGL format v1  |
| `0x12340002` | BGL format v2  |

### Gzip Offset

Byte offset from the beginning of the file where the gzip-compressed stream
starts. Must be >= 6 (header size) and < file size.

---

## 2. Block Structure

All data within the gzip stream is organized as sequential blocks. Each block
has a variable-length header followed by payload data.

```
=== Block Layout ===

+--------+------------------+------------------------+
| Byte 0 |  Extra length    |      Data              |
| (type +|  bytes           |      (N bytes)         |
|  len)  |  (0-4 bytes)     |                        |
+--------+------------------+------------------------+
         ^                  ^
         |                  |
         if length_code < 4: (length_code + 1) bytes
         if length_code >= 4: no extra bytes, N = length_code - 4
```

Byte 0 encodes both the block type and length in 4 bits each:

```
Byte 0:
  7   6   5   4   3   2   1   0
+---+---+---+---+---+---+---+---+
| N3| N2| N1| N0| T3| T2| T1| T0|
+---+---+---+---+---+---+---+---+
|<-length code->|<-block type ->|
```

- `block_type` = `byte & 0x0F`
- `length_code` = `byte >> 4`

### 2.1 Length Encoding

| length_code | Meaning                              | Extra bytes | Max size           |
|-------------|--------------------------------------|-------------|--------------------|
| 0           | 1 additional big-endian byte         | 1           | 255                |
| 1           | 2 additional big-endian bytes        | 2           | 65,535             |
| 2           | 3 additional big-endian bytes        | 3           | 16,777,215         |
| 3           | 4 additional big-endian bytes        | 4           | 4,294,967,295      |
| 4 - 15      | Length = `length_code - 4` (inline)  | 0           | 0 - 11             |

### 2.2 Block Types

| Type | Description              |
|------|--------------------------|
| 0    | Extended metadata        |
| 1    | Dictionary entry (standard) |
| 2    | Embedded resource        |
| 3    | Info field               |
| 4    | EOF marker               |
| 6    | Entries section start    |
| 7    | Dictionary entry (same format as type 1) |
| 10   | Dictionary entry (same format as type 1) |
| 11   | Dictionary entry (different format) |
| 13   | Dictionary entry (same format as type 1) |

### 2.3 Full-Stream Scan Requirement

The **Type 6** (entries section start) block exists in the format but is **not
reliable** for positioning the entries section. In practice, many BGL files do
not follow the expected block ordering -- info blocks (Type 3) can appear
anywhere in the stream, including interspersed among entries and after resources.

Because of this, the implementation must perform a **full scan of the entire
gzip stream** on first access. During this scan:

1. **All Type 3 blocks** are collected to build the complete metadata (title,
   author, encoding, flags, etc.).
2. **The first entry-type block** encountered is recorded as the entries start
   offset, rather than relying on the Type 6 marker.
3. **The first Type 2 block** encountered is recorded as the resources start
   offset.
4. **Entry and resource counts** are tallied from actual blocks seen.

This means the initial metadata loading requires decompressing the entire file
once. Use `bgl_reader_prepare()` to trigger this scan explicitly (e.g., on a
background thread) if you want to control timing. Otherwise it happens lazily
on first access.

---

## 3. Type 0 - Extended Metadata

Data begins with a 1-byte sub-type code:

| Offset | Size | Field | Description       |
|--------|------|-------|-------------------|
| 0      | 1    | code  | Sub-type code     |
| 1      | N    | data  | Depends on code   |

### Sub-type Codes

| Code | Description              | Data Format              |
|------|--------------------------|--------------------------|
| 2    | Approximate entry count  | (parsed but not stored)  |
| 8    | Default charset          | 1 byte: charset code     |

The default charset (code 8) is used as a fallback encoding when source/target
charsets are not specified in Type 3 info blocks. See [Charset Code Table](#charset-code-table).

---

## 4. Type 3 - Info Field

Each Type 3 block contains exactly **one** metadata field.

| Offset | Size | Field  | Description                    |
|--------|------|--------|--------------------------------|
| 0      | 2    | code   | Field code (big-endian uint16) |
| 2      | N    | value  | Field value (format varies)    |

If the value bytes are all zero, the block is skipped.

### Info Field Codes

| Code  | Name                    | Value Format    | Description              |
|-------|-------------------------|-----------------|--------------------------|
| 0x01  | Title                   | Raw string      | Dictionary title         |
| 0x02  | Author                  | Raw string      | Author                   |
| 0x03  | Email                   | Raw string      | Author email             |
| 0x04  | Copyright               | Raw string      | Copyright information    |
| 0x07  | Source language         | uint32 BE       | Language code            |
| 0x08  | Target language         | uint32 BE       | Language code            |
| 0x09  | Description             | Raw string      | Description              |
| 0x0a  | Browsing enabled        | -               | (not parsed)             |
| 0x0b  | Icon 1                  | -               | (not parsed)             |
| 0x0c  | Number of entries       | uint32 BE       | Entry count              |
| 0x11  | Flags                   | uint32 BE       | Encoding/feature flags   |
| 0x14  | Creation time           | uint32 BE       | Timestamp (minutes)      |
| 0x1a  | Source charset          | uint8           | Charset code             |
| 0x1b  | Target charset          | uint8           | Charset code             |
| 0x1c  | First updated           | -               | (not parsed)             |
| 0x20  | Case sensitive 2        | -               | (not parsed)             |
| 0x24  | Icon 2                  | -               | (not parsed)             |
| 0x2c  | Purchase license msg    | -               | (not parsed)             |
| 0x2d  | License expired msg     | -               | (not parsed)             |
| 0x2e  | Purchase address        | -               | (not parsed)             |
| 0x30  | Title (wide)            | Raw string      | (not parsed)             |
| 0x31  | Author (wide)           | Raw string      | (not parsed)             |
| 0x33  | Last updated            | -               | (not parsed)             |
| 0x3b  | Contractions            | -               | (not parsed)             |
| 0x3d  | Font name               | -               | (not parsed)             |
| 0x41  | About                   | Raw string      | (not parsed)             |
| 0x43  | Substring match length  | -               | (not parsed)             |

### Flags Field (Code 0x11)

| Bit(s) | Mask     | Flag                  | Meaning                        |
|--------|----------|-----------------------|--------------------------------|
| 12     | 0x1000   | case_sensitive        | Set = case sensitive           |
| 15     | 0x8000   | utf8_encoding         | Set = UTF-8 mode               |
| 16     | 0x10000  | spelling_alternatives | Clear = has spelling alts (inverted logic) |

### Timestamp (Code 0x14)

The value represents minutes since 1970-01-01 (Julian Day 2440588):

```
total_minutes = uint32_value
days    = total_minutes / (24 * 60)    // offset from 1970-01-01
hours   = (total_minutes % (24 * 60)) / 60
minutes = (total_minutes % (24 * 60)) % 60
```

---

## 5. Dictionary Entry Format

### 5.1 Standard Entry (Block Types 1, 7, 10, 13)

| Offset | Size        | Field              | Description                      |
|--------|-------------|--------------------|----------------------------------|
| 0      | 1           | word_len           | Word length (uint8)              |
| 1      | word_len    | word               | Headword (source_encoding)       |
| 1+W    | 2           | defi_len           | Definition length (uint16 BE)    |
| 3+W    | defi_len    | definition         | Definition data (see below)      |
| 3+W+D  | 1           | alt_len            | First alternate length (uint8)   |
| 4+W+D  | alt_len     | alternate          | First alternate word             |
| ...    | ...         | ...                | More alternates (0 = end)        |

Alternates are read sequentially. Each alternate is prefixed by a 1-byte length.
An `alt_len` of 0 signals the end of the alternates list.

### 5.2 Type 11 Entry

Uses 4-byte big-endian lengths. Alternates come **before** the definition.

| Offset | Size        | Field              | Description                      |
|--------|-------------|--------------------|----------------------------------|
| 0      | 1           | flag               | Flag byte (skipped)              |
| 1      | 4           | word_len           | Word length (uint32 BE)          |
| 5      | word_len    | word               | Headword (source_encoding)       |
| 5+W    | 4           | alts_count         | Number of alternates (uint32 BE) |
| 9+W    | ...         | alternates         | See below                        |
| ...    | 4           | defi_len           | Definition length (uint32 BE)    |
| ...    | defi_len    | definition         | Definition data                  |

Each alternate in the alternates section:

| Size | Field     | Description                        |
|------|-----------|------------------------------------|
| 4    | alt_len   | Alternate length (uint32 BE)       |
| N    | alternate | Alternate word (source_encoding)  |

---

## 6. Definition Data Format

The definition field is a byte stream containing HTML/text interleaved with
control codes that encode structured metadata.

### 6.1 Structure

```
[definition body text] [0x14 separator] [field_code field_data ...]
```

The body and fields are separated by `0x14`. If `0x14` is followed by a space
(`0x20`), it is treated as part of the body text, not a separator. If no
separator is found, the entire data is body text.

### 6.2 Control Codes

| Code   | Name                | Format                                           |
|--------|---------------------|--------------------------------------------------|
| 0x02   | Part-of-speech      | `0x02 <pos_code>` (2 bytes)                       |
| 0x14   | Field separator     | Separates body from metadata fields               |
| 0x18   | Entry title         | `0x18 <1 byte len> <title>`                       |
| 0x1A   | Unknown (Hebrew)    | `0x1A <1 byte len> <data>`                        |
| 0x28   | Title transcription | `0x28 <2 bytes len BE> <text>`                    |
| 0x40-0x4F | Numeric/text     | `0x4X` (length = `code - 0x3F`, data follows)    |
| 0x50-0x5F | Transcription    | `0x5X <trans_code> <1 byte len> <data>`           |
| 0x60   | Transcription (long)| `0x60 <trans_code> <2 bytes len BE> <data>`      |

#### POS Marker (0x02)

The byte following `0x02` must be in range `0x30-0x47`. If not, it is treated
as a new field code.

#### Numeric/Text Fields (0x40-0x4F)

The field code itself encodes the data length:

```
code = 0x40 -> length = 1 byte follows
code = 0x41 -> length = 2 bytes follow
...
code = 0x4F -> length = 16 bytes follow
```

#### Transcription Fields (0x50-0x5F, 0x60)

A `trans_code` byte follows the field code. Only `trans_code == 0x1B` is
processed; other codes are skipped (length and data are still consumed).

### 6.3 POS Code Table

| Code | Name              | Abbreviation |
|------|-------------------|--------------|
| 0x30 | noun              | n.           |
| 0x31 | adjective         | adj.         |
| 0x32 | verb              | v.           |
| 0x33 | adverb            | adv.         |
| 0x34 | interjection      | interj.      |
| 0x35 | pronoun           | pron.        |
| 0x36 | preposition       | prep.        |
| 0x37 | conjunction       | conj.        |
| 0x38 | suffix            | suff.        |
| 0x39 | prefix            | pref.        |
| 0x3A | article           | art.         |
| 0x3B | (unknown)         |              |
| 0x3C | abbreviation      | abbr.        |
| 0x3D | masc noun/adj     |              |
| 0x3E | fem noun/adj      |              |
| 0x3F | masc+fem noun/adj |              |
| 0x40 | feminine noun     | f. n.        |
| 0x41 | masc+fem noun     |              |
| 0x42 | masculine noun    | m. n.        |
| 0x43 | numeral           | num.         |
| 0x44 | participle        | part.        |
| 0x45-0x47 | (unknown)     |              |

---

## 7. Charset Tags in Definition Text

Definition body text and certain fields (title, title_trans, transcription) may
contain inline charset switching tags:

```
<charset c="U">text</charset>   UTF-8
<charset c="K">text</charset>   Source language encoding
<charset c="E">text</charset>   Source language encoding
<charset c="G">text</charset>   GBK
<charset c="T">text</charset>   Babylon character references
```

Tags are detected case-insensitively. Nesting is supported (max depth 32).

### Babylon Character References (type T)

Semicolon-separated hexadecimal Unicode code points:

```
00E6;00E7;00E8;
```

Each value (4+ hex digits before `;`) is a Unicode code point converted to UTF-8.

---

## 8. Resource Format (Block Type 2)

| Offset | Size | Field      | Description            |
|--------|------|------------|------------------------|
| 0      | 1    | name_len   | Name length (uint8)    |
| 1      | N    | name       | Resource name (ASCII)  |
| 1+N    | ...  | data       | Raw resource data      |

The name is used as-is (no encoding conversion). Remaining bytes are the raw
resource content (e.g., PNG image data, HTML, etc.).

---

## 9. Encoding Detection

Encoding is determined by the following priority cascade:

| Priority | Source                                    | Result                    |
|----------|-------------------------------------------|---------------------------|
| 1        | UTF-8 flag (info code 0x11, bit 0x8000)  | source = target = "UTF-8" |
| 2        | Source/target charset (info codes 0x1a/0x1b) | Mapped via charset table |
| 3        | Language-based (info codes 0x07/0x08)     | Language's default encoding |
| 4        | Default charset (type 0, code 8)          | Mapped via charset table  |
| 5        | Hardcoded fallback                        | "CP1252"                  |

The `default_encoding` (from type 0 code 8) is also used as the fallback for
text segments within definitions that are not inside any `<charset>` tag.

---

## 10. Control Characters

### Removed from definition text

| Range     | Characters     |
|-----------|----------------|
| 0x00-0x08 | Control chars  |
| 0x0C      | Form feed      |
| 0x0E-0x1F | Control chars  |

### Preserved

| Value | Character  |
|-------|------------|
| 0x09  | Tab        |
| 0x0A  | Line feed  |
| 0x0B  | Vertical tab |
| 0x0D  | Carriage return |

### Image link delimiters

BGL image tags use RS/US to delimit `src` values:

```
<IMG src='\x1eimage.png\x1f'>
```

| Value | Name             | Role              |
|-------|------------------|-------------------|
| 0x1E  | Record Separator | Start delimiter   |
| 0x1F  | Unit Separator   | End delimiter     |

---

## 11. Language Code Table

Used by info field codes 0x07 (source) and 0x08 (target).
Value is a 4-byte big-endian uint32.

| Code | Name                      | Encoding |
|------|---------------------------|----------|
| 0x00 | English                   | CP1252   |
| 0x01 | French                    | CP1252   |
| 0x02 | Italian                   | CP1252   |
| 0x03 | Spanish                   | CP1252   |
| 0x04 | Dutch                     | CP1252   |
| 0x05 | Portuguese                | CP1252   |
| 0x06 | German                    | CP1252   |
| 0x07 | Russian                   | CP1251   |
| 0x08 | Japanese                  | CP932    |
| 0x09 | Chinese (Traditional)     | CP950    |
| 0x0A | Chinese (Simplified)      | CP936    |
| 0x0B | Greek                     | CP1253   |
| 0x0C | Korean                    | CP949    |
| 0x0D | Turkish                   | CP1254   |
| 0x0E | Hebrew                    | CP1255   |
| 0x0F | Arabic                    | CP1256   |
| 0x10 | Thai                      | CP874    |
| 0x11 | Other                     | CP1252   |
| 0x12 | Chinese (Simplified alt)  | CP936    |
| 0x13 | Chinese (Traditional alt) | CP950   |
| 0x14 | Eastern European (other)  | CP1250   |
| 0x15 | Western European (other)  | CP1252   |
| 0x16 | Russian (other)           | CP1251   |
| 0x17 | Japanese (other)          | CP932    |
| 0x18 | Baltic (other)            | CP1257   |
| 0x19 | Greek (other)             | CP1253   |
| 0x1A | Korean (other)            | CP949    |
| 0x1B | Turkish (other)           | CP1254   |
| 0x1C | Thai (other)              | CP874    |
| 0x1D | Polish                    | CP1250   |
| 0x1E | Hungarian                 | CP1250   |
| 0x1F | Czech                     | CP1250   |
| 0x20 | Lithuanian                | CP1257   |
| 0x21 | Latvian                   | CP1257   |
| 0x22 | Catalan                   | CP1252   |
| 0x23 | Croatian                  | CP1250   |
| 0x24 | Serbian                   | CP1250   |
| 0x25 | Slovak                    | CP1250   |
| 0x26 | Albanian                  | CP1252   |
| 0x27 | Urdu                      | CP1256   |
| 0x28 | Slovenian                 | CP1250   |
| 0x29 | Estonian                  | CP1252   |
| 0x2A | Bulgarian                 | CP1250   |
| 0x2B | Danish                    | CP1252   |
| 0x2C | Finnish                   | CP1252   |
| 0x2D | Icelandic                 | CP1252   |
| 0x2E | Norwegian                 | CP1252   |
| 0x2F | Romanian                  | CP1252   |
| 0x30 | Swedish                   | CP1252   |
| 0x31 | Ukrainian                 | CP1251   |
| 0x32 | Belarusian                | CP1251   |
| 0x33 | Persian (Farsi)           | CP1256   |
| 0x34 | Basque                    | CP1252   |
| 0x35 | Macedonian                | CP1250   |
| 0x36 | Afrikaans                 | CP1252   |
| 0x37 | Faroese                   | CP1252   |
| 0x38 | Latin                     | CP1252   |
| 0x39 | Esperanto                 | CP1254   |
| 0x3A | Tamazight                 | CP1252   |
| 0x3B | Armenian                  | CP1252   |
| 0x3C | Hindi                     | CP1252   |
| 0x3D | Somali                    | CP1252   |

---

## 12. Charset Code Table

Used by info field codes 0x1a (source) and 0x1b (target), and type 0 sub-code 8
(default charset). Value is a single uint8.

| Code | Encoding | Description          |
|------|----------|----------------------|
| 0x41 | CP1252   | Default / Latin      |
| 0x42 | CP1252   | Latin                |
| 0x43 | CP1250   | Eastern European     |
| 0x44 | CP1251   | Cyrillic             |
| 0x45 | CP932    | Japanese             |
| 0x46 | CP950    | Traditional Chinese  |
| 0x47 | CP936    | Simplified Chinese   |
| 0x48 | CP1257   | Baltic               |
| 0x49 | CP1253   | Greek                |
| 0x4A | CP949    | Korean               |
| 0x4B | CP1254   | Turkish              |
| 0x4C | CP1255   | Hebrew               |
| 0x4D | CP1256   | Arabic               |
| 0x4E | CP874    | Thai                 |

Default fallback: **CP1252**.
