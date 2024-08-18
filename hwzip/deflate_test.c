/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "deflate.h"

#include <stdlib.h>
#include <string.h>
#include "bitstream.h"
#include "hamlet.h"
#include "huffman.h"
#include "test_utils.h"
#include "zlib_utils.h"

/* Compress src, then decompress it and check that it matches.
   Returns the size of the compressed data. */
static size_t deflate_roundtrip(const uint8_t *src, size_t len)
{
        uint8_t *compressed, *decompressed;
        size_t compressed_sz, decompressed_sz, compressed_used;
        size_t i, tmp;

        compressed = malloc(len * 2 + 100);
        CHECK(hwdeflate(src, len, compressed, 2 * len + 100, &compressed_sz));

        decompressed = malloc(len);
        CHECK(hwinflate(compressed, compressed_sz, &compressed_used,
                        decompressed, len, &decompressed_sz) == HWINF_OK);
        CHECK(compressed_used == compressed_sz);
        CHECK(decompressed_sz == len);
        CHECK(memcmp(src, decompressed, len) == 0);

        if (len < 1000) {
                /* For small inputs, check that a too small buffer fails. */
                for (i = 0; i < compressed_used; i++) {
                        CHECK(!hwdeflate(src, len, compressed, i, &tmp));
                }
        } else if (compressed_sz > 500) {
                /* For larger inputs, try cutting off the first block. */
                CHECK(!hwdeflate(src, len, compressed, 500, &tmp));
        }

        free(compressed);
        free(decompressed);

        return compressed_sz;
}

typedef enum {
        UNCOMP = 0x0,
        STATIC = 0x1,
        DYNAMIC = 0x2
} block_t;

static void check_deflate_string(const char *str, block_t expected_type)
{
        uint8_t comp[1000];
        size_t comp_sz;

        CHECK(hwdeflate((const uint8_t*)str, strlen(str), comp,
                        sizeof(comp), &comp_sz));
        CHECK(((comp[0] & 7) >> 1) == expected_type);

        deflate_roundtrip((const uint8_t*)str, strlen(str));
}

void test_deflate_basic(void)
{
        char buf[256];
        size_t i;

        /* Empty input; a static block is shortest. */
        check_deflate_string("", STATIC);

        /* One byte; a static block is shortest. */
        check_deflate_string("a", STATIC);

        /* Repeated substring. */
        check_deflate_string("hellohello", STATIC);

        /* Non-repeated long string with small alphabet. Dynamic. */
        check_deflate_string("abcdefghijklmnopqrstuvwxyz"
                             "zyxwvutsrqponmlkjihgfedcba", DYNAMIC);

        /* No repetition, uniform distribution. Uncompressed. */
        for (i = 0; i < 255; i++) {
                buf[i] = (char)(i + 1);
        }
        buf[255] = 0;
        check_deflate_string(buf, UNCOMP);
}

/* PKZIP 2.50    pkzip -exx a.zip hamlet.txt             79754 bytes
   info-zip 3.0  zip -9 a.zip hamlet.txt                 80032 bytes
   7-Zip 16.02   7z a -mx=9 -mpass=15 a.zip hamlet.txt   76422 bytes */
void test_deflate_hamlet(void)
{
        size_t len;

        len = deflate_roundtrip(hamlet, sizeof(hamlet));

        /* Update if we make compression better. */
        CHECK(len == 79708);
}

void test_deflate_mixed_blocks(void)
{
        uint8_t *src, *p;
        uint32_t r;
        size_t i, j;
        const size_t src_size = 2 * 1024 * 1024;

        src = malloc(src_size);

        memset(src, 0, src_size);

        p = src;
        r = 0;
        for (i = 0; i < 5; i++) {
                /* Data suitable for compressed blocks. */
                memcpy(src, hamlet, sizeof(hamlet));
                p += sizeof(hamlet);

                /* Random data, likely to go in an uncompressed block. */
                for (j = 0; j < 128000; j++) {
                        r = next_test_rand(r);
                        *p++ = (uint8_t)(r >> 24);
                }

                assert((size_t)(p - src) <= src_size);
        }

        deflate_roundtrip(src, src_size);

        free(src);
}

void test_deflate_random(void)
{
        uint8_t *src;
        const size_t src_size = 3 * 1024 * 1024;
        uint32_t r;
        size_t i;

        src = malloc(src_size);

        r = 0;
        for (i = 0; i < src_size; i++) {
                r = next_test_rand(r);
                src[i] = (uint8_t)(r >> 24);
        }

        deflate_roundtrip(src, src_size);

        free(src);
}


#define MIN_LITLEN_LENS 257
#define MAX_LITLEN_LENS 288
#define MIN_DIST_LENS 1
#define MAX_DIST_LENS 32
#define MIN_CODELEN_LENS 4
#define MAX_CODELEN_LENS 19
#define CODELEN_MAX_LIT 15
#define CODELEN_COPY 16
#define CODELEN_COPY_MIN 3
#define CODELEN_COPY_MAX 6
#define CODELEN_ZEROS 17
#define CODELEN_ZEROS_MIN 3
#define CODELEN_ZEROS_MAX 10
#define CODELEN_ZEROS2 18
#define CODELEN_ZEROS2_MIN 11
#define CODELEN_ZEROS2_MAX 138

struct block_header {
        bool bfinal;
        int type;
        size_t num_litlen_lens;
        size_t num_dist_lens;
        int code_lengths[MIN_LITLEN_LENS + MAX_LITLEN_LENS];
};

static const int codelen_lengths_order[MAX_CODELEN_LENS] =
{ 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

static struct block_header read_block_header(istream_t *is)
{
        struct block_header h;
        uint64_t bits;
        size_t num_codelen_lens, used, i, n;
        uint8_t codelen_lengths[MAX_CODELEN_LENS];
        huffman_decoder_t codelen_dec;
        int sym;

        bits = istream_bits(is);
        h.bfinal = bits & 1;
        bits >>= 1;

        h.type = (int)lsb(bits, 2);
        istream_advance(is, 3);

        if (h.type != 2) {
                return h;
        }

        bits = istream_bits(is);

        /* Number of litlen codeword lengths (5 bits + 257). */
        h.num_litlen_lens = (size_t)(lsb(bits, 5) + MIN_LITLEN_LENS);
        bits >>= 5;
        assert(h.num_litlen_lens <= MAX_LITLEN_LENS);

        /* Number of dist codeword lengths (5 bits + 1). */
        h.num_dist_lens = (size_t)(lsb(bits, 5) + MIN_DIST_LENS);
        bits >>= 5;
        assert(h.num_dist_lens <= MAX_DIST_LENS);

        /* Number of code length lengths (4 bits + 4). */
        num_codelen_lens = (size_t)(lsb(bits, 4) + MIN_CODELEN_LENS);
        bits >>= 4;
        assert(num_codelen_lens <= MAX_CODELEN_LENS);

        istream_advance(is, 5 + 5 + 4);


        /* Read the codelen codeword lengths (3 bits each)
           and initialize the codelen decoder. */
        for (i = 0; i < num_codelen_lens; i++) {
                bits = istream_bits(is);
                codelen_lengths[codelen_lengths_order[i]] =
                        (uint8_t)lsb(bits, 3);
                istream_advance(is, 3);
        }
        for (; i < MAX_CODELEN_LENS; i++) {
                codelen_lengths[codelen_lengths_order[i]] = 0;
        }
        huffman_decoder_init(&codelen_dec, codelen_lengths, MAX_CODELEN_LENS);


        /* Read the litlen and dist codeword lengths. */
        i = 0;
        while (i < h.num_litlen_lens + h.num_dist_lens) {
                bits = istream_bits(is);
                sym = huffman_decode(&codelen_dec, (uint16_t)bits, &used);
                bits >>= used;
                istream_advance(is, used);
                assert(sym != -1);

                if (sym >= 0 && sym <= CODELEN_MAX_LIT) {
                        /* A literal codeword length. */
                        h.code_lengths[i++] = (uint8_t)sym;
                } else if (sym == CODELEN_COPY) {
                        /* Copy the previous codeword length 3--6 times. */
                        /* 2 bits + 3 */
                        n = (size_t)lsb(bits, 2) + CODELEN_COPY_MIN;
                        istream_advance(is, 2);
                        assert(n >= CODELEN_COPY_MIN && n <= CODELEN_COPY_MAX);
                        while (n--) {
                                h.code_lengths[i] = h.code_lengths[i - 1];
                                i++;
                        }
                } else if (sym == CODELEN_ZEROS) {
                        /* 3--10 zeros; 3 bits + 3 */
                        n = (size_t)(lsb(bits, 3) + CODELEN_ZEROS_MIN);
                        istream_advance(is, 3);
                        assert(n >= CODELEN_ZEROS_MIN &&
                               n <= CODELEN_ZEROS_MAX);
                        while (n--) {
                                h.code_lengths[i++] = 0;
                        }
                } else if (sym == CODELEN_ZEROS2) {
                        /* 11--138 zeros; 7 bits + 11. */
                        n = (size_t)(lsb(bits, 7) + CODELEN_ZEROS2_MIN);
                        istream_advance(is, 7);
                        assert(n >= CODELEN_ZEROS2_MIN &&
                               n <= CODELEN_ZEROS2_MAX);
                        while (n--) {
                                h.code_lengths[i++] = 0;
                        }
                }
        }

        return h;
}

void test_deflate_no_dist_codes(void)
{
        /* Compressing this will not use any dist codes, but check that we
           still encode two non-zero dist codes to be compatible with old
           zlib versions. */
        static const uint8_t src[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        };

        uint8_t compressed[1000];
        size_t compressed_sz;
        istream_t is;
        struct block_header h;

        CHECK(hwdeflate(src, sizeof(src),
                        compressed, sizeof(compressed), &compressed_sz));

        istream_init(&is, compressed, compressed_sz);
        h = read_block_header(&is);

        CHECK(h.num_dist_lens == 2);
        CHECK(h.code_lengths[h.num_litlen_lens + 0] == 1);
        CHECK(h.code_lengths[h.num_litlen_lens + 1] == 1);
}


void test_inflate_invalid_block_header(void)
{
        /* bfinal: 0, btype: 11 */
        static const uint8_t src = 0x6; /* 0000 0110 */
        size_t src_used, dst_used;
        uint8_t dst[10];

        CHECK(hwinflate(&src, 1, &src_used, dst, 10, &dst_used) == HWINF_ERR);
}

void test_inflate_uncompressed(void)
{
        uint8_t dst[10];
        size_t src_used, dst_used;

        static const uint8_t bad[] = {
                0x01,           /* 0000 0001  bfinal: 1, btype: 00 */
                0x05, 0x00,     /* len: 5 */
                0x12, 0x34      /* nlen: garbage */
        };

        static const uint8_t good[] = {
                0x01,           /* 0000 0001  bfinal: 1, btype: 00 */
                0x05, 0x00,     /* len: 5 */
                0xfa, 0xff,     /* nlen */
                'H', 'e', 'l', 'l', 'o'
        };


        /* Too short for block header. */
        CHECK(hwinflate(bad, 0, &src_used, dst, 10, &dst_used) == HWINF_ERR);
        /* Too short for len. */
        CHECK(hwinflate(bad, 1, &src_used, dst, 10, &dst_used) == HWINF_ERR);
        CHECK(hwinflate(bad, 2, &src_used, dst, 10, &dst_used) == HWINF_ERR);
        /* Too short for nlen. */
        CHECK(hwinflate(bad, 3, &src_used, dst, 10, &dst_used) == HWINF_ERR);
        CHECK(hwinflate(bad, 4, &src_used, dst, 10, &dst_used) == HWINF_ERR);
        /* nlen len mismatch. */
        CHECK(hwinflate(bad, 5, &src_used, dst, 10, &dst_used) == HWINF_ERR);

        /* Not enough input. */
        CHECK(hwinflate(good, 9, &src_used, dst, 4, &dst_used) == HWINF_ERR);

        /* Not enough room to output. */
        CHECK(hwinflate(good, 10, &src_used, dst, 4, &dst_used) == HWINF_FULL);

        /* Success. */
        CHECK(hwinflate(good, 10, &src_used, dst, 5, &dst_used) == HWINF_OK);
        CHECK(src_used == 10);
        CHECK(dst_used == 5);
        CHECK(memcmp(dst, "Hello", 5) == 0);
}

void test_inflate_twocities_intro(void)
{
        static const uint8_t deflated[] = {
0x74,0xeb,0xcd,0x0d,0x80,0x20,0x0c,0x47,0x71,0xdc,0x9d,0xa2,0x03,0xb8,0x88,0x63,
0xf0,0xf1,0x47,0x9a,0x00,0x35,0xb4,0x86,0xf5,0x0d,0x27,0x63,0x82,0xe7,0xdf,0x7b,
0x87,0xd1,0x70,0x4a,0x96,0x41,0x1e,0x6a,0x24,0x89,0x8c,0x2b,0x74,0xdf,0xf8,0x95,
0x21,0xfd,0x8f,0xdc,0x89,0x09,0x83,0x35,0x4a,0x5d,0x49,0x12,0x29,0xac,0xb9,0x41,
0xbf,0x23,0x2e,0x09,0x79,0x06,0x1e,0x85,0x91,0xd6,0xc6,0x2d,0x74,0xc4,0xfb,0xa1,
0x7b,0x0f,0x52,0x20,0x84,0x61,0x28,0x0c,0x63,0xdf,0x53,0xf4,0x00,0x1e,0xc3,0xa5,
0x97,0x88,0xf4,0xd9,0x04,0xa5,0x2d,0x49,0x54,0xbc,0xfd,0x90,0xa5,0x0c,0xae,0xbf,
0x3f,0x84,0x77,0x88,0x3f,0xaf,0xc0,0x40,0xd6,0x5b,0x14,0x8b,0x54,0xf6,0x0f,0x9b,
0x49,0xf7,0xbf,0xbf,0x36,0x54,0x5a,0x0d,0xe6,0x3e,0xf0,0x9e,0x29,0xcd,0xa1,0x41,
0x05,0x36,0x48,0x74,0x4a,0xe9,0x46,0x66,0x2a,0x19,0x17,0xf4,0x71,0x8e,0xcb,0x15,
0x5b,0x57,0xe4,0xf3,0xc7,0xe7,0x1e,0x9d,0x50,0x08,0xc3,0x50,0x18,0xc6,0x2a,0x19,
0xa0,0xdd,0xc3,0x35,0x82,0x3d,0x6a,0xb0,0x34,0x92,0x16,0x8b,0xdb,0x1b,0xeb,0x7d,
0xbc,0xf8,0x16,0xf8,0xc2,0xe1,0xaf,0x81,0x7e,0x58,0xf4,0x9f,0x74,0xf8,0xcd,0x39,
0xd3,0xaa,0x0f,0x26,0x31,0xcc,0x8d,0x9a,0xd2,0x04,0x3e,0x51,0xbe,0x7e,0xbc,0xc5,
0x27,0x3d,0xa5,0xf3,0x15,0x63,0x94,0x42,0x75,0x53,0x6b,0x61,0xc8,0x01,0x13,0x4d,
0x23,0xba,0x2a,0x2d,0x6c,0x94,0x65,0xc7,0x4b,0x86,0x9b,0x25,0x3e,0xba,0x01,0x10,
0x84,0x81,0x28,0x80,0x55,0x1c,0xc0,0xa5,0xaa,0x36,0xa6,0x09,0xa8,0xa1,0x85,0xf9,
0x7d,0x45,0xbf,0x80,0xe4,0xd1,0xbb,0xde,0xb9,0x5e,0xf1,0x23,0x89,0x4b,0x00,0xd5,
0x59,0x84,0x85,0xe3,0xd4,0xdc,0xb2,0x66,0xe9,0xc1,0x44,0x0b,0x1e,0x84,0xec,0xe6,
0xa1,0xc7,0x42,0x6a,0x09,0x6d,0x9a,0x5e,0x70,0xa2,0x36,0x94,0x29,0x2c,0x85,0x3f,
0x24,0x39,0xf3,0xae,0xc3,0xca,0xca,0xaf,0x2f,0xce,0x8e,0x58,0x91,0x00,0x25,0xb5,
0xb3,0xe9,0xd4,0xda,0xef,0xfa,0x48,0x7b,0x3b,0xe2,0x63,0x12,0x00,0x00,0x20,0x04,
0x80,0x70,0x36,0x8c,0xbd,0x04,0x71,0xff,0xf6,0x0f,0x66,0x38,0xcf,0xa1,0x39,0x11,
0x0f };
        static const uint8_t expected[] =
"It was the best of times,\n"
"it was the worst of times,\n"
"it was the age of wisdom,\n"
"it was the age of foolishness,\n"
"it was the epoch of belief,\n"
"it was the epoch of incredulity,\n"
"it was the season of Light,\n"
"it was the season of Darkness,\n"
"it was the spring of hope,\n"
"it was the winter of despair,\n"
"\n"
"we had everything before us, we had nothing before us, "
"we were all going direct to Heaven, we were all going direct the other way"
"---in short, the period was so far like the present period, "
"that some of its noisiest authorities insisted on its being received, "
"for good or for evil, in the superlative degree of comparison only.\n";

        uint8_t dst[1000];
        size_t src_used, dst_used, i;

        CHECK(hwinflate(deflated, sizeof(deflated), &src_used,
                        dst, sizeof(dst), &dst_used) == HWINF_OK);
        CHECK(dst_used == sizeof(expected));
        CHECK(src_used == sizeof(deflated));
        CHECK(memcmp(dst, expected, sizeof(expected)) == 0);

        /* Truncated inputs should fail. */
        for (i = 0; i < sizeof(deflated); i++) {
                CHECK(hwinflate(deflated, i, &src_used, dst, sizeof(dst),
                                &dst_used) == HWINF_ERR);
        }
}

void test_inflate_hamlet(void)
{
        static uint8_t compressed[sizeof(hamlet) * 2];
        static uint8_t decompressed[sizeof(hamlet)];

        int level;
        size_t compressed_sz, src_used, dst_used;

        for (level = 0; level <= 9; level++) {
                compressed_sz = zlib_deflate(hamlet, sizeof(hamlet),
                                             level, compressed,
                                             sizeof(compressed));

                CHECK(hwinflate(compressed, compressed_sz, &src_used,
                                decompressed, sizeof(decompressed),
                                &dst_used) == HWINF_OK);
                CHECK(src_used == compressed_sz);
                CHECK(dst_used == sizeof(hamlet));
                CHECK(memcmp(decompressed, hamlet, sizeof(hamlet)) == 0);
        }
}

/* Both hwzip and zlib will always emit at least two non-zero dist codeword
   lengths, but that's not required by the spec. Make sure we can inflate
   also when there are no dist codes. */
void test_inflate_no_dist_codes(void)
{
        static const uint8_t compressed[] = {
        0x05, 0x00, 0x05, 0x0d, 0x00, 0x30, 0xe8, 0x38, 0x4e, 0xff, 0xb6, 0xdf,
        0x03, 0x24, 0x16, 0x35, 0x8f, 0xac, 0x9e, 0xbd, 0xdb, 0xe9, 0xca, 0x70,
        0x53, 0x61, 0x42, 0x78, 0x1f
        };

        static const uint8_t expected[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        };

        uint8_t decompressed[sizeof(expected)];
        size_t compressed_used, decompressed_used;

        CHECK(hwinflate(compressed, sizeof(compressed), &compressed_used,
                        decompressed, sizeof(decompressed), &decompressed_used)
                        == HWINF_OK);
        CHECK(decompressed_used == sizeof(expected));
        CHECK(memcmp(decompressed, expected, sizeof(expected)) == 0);
}
