/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "hamlet.h"
#include "huffman.h"

#define BITSTREAM_LEN (64 * 1024 -1)

struct decode_data {
        /* A stream of Huffman encoded symbols. */
        uint16_t bits[BITSTREAM_LEN];
        int sym[BITSTREAM_LEN];
        uint8_t len[BITSTREAM_LEN];

        huffman_decoder_t dec;
};

void *bench_huffman_decode_setup(void)
{
        uint16_t freqs[256] = {0};
        size_t i;
        huffman_encoder_t enc;
        struct decode_data *dd;
        bool ok;

        dd = malloc(sizeof(*dd));

        /* Create the stream of encoded symbols. */
        for (i = 0; i < BITSTREAM_LEN; i++) {
                freqs[hamlet[i]]++;
        }
        huffman_encoder_init(&enc, freqs, 256, 15);
        for (i = 0; i < BITSTREAM_LEN; i++) {
                dd->bits[i] = enc.codewords[hamlet[i]];
                dd->sym[i] = hamlet[i];
                dd->len[i] = enc.lengths[hamlet[i]];
        }

        ok = huffman_decoder_init(&dd->dec, enc.lengths, 256);
        assert(ok);
        (void)ok;

        return dd;
}

void bench_huffman_decode(void *aux)
{
        volatile int sym; /* Volatile so the result isn't optimized away. */
        size_t i, len;
        struct decode_data *dd = aux;

        for (i = 0; i < BITSTREAM_LEN; i++) {
                sym = huffman_decode(&dd->dec, dd->bits[i], &len);
                assert(sym == dd->sym[i]);
                assert(len == dd->len[i]);
                (void)sym;
        }
}

void bench_huffman_decode_teardown(void *aux)
{
        free(aux);
}

size_t bench_huffman_decode_bytes(void)
{
        size_t i, num_bits;
        struct decode_data *dd;

        dd = bench_huffman_decode_setup();

        num_bits = 0;
        for (i = 0; i < BITSTREAM_LEN; i++) {
                num_bits += dd->len[i];
        }

        free(dd);

        return num_bits / 8;
}


void *bench_huffman_encoder_init_setup(void)
{
        return NULL;
}

void bench_huffman_encoder_init(void *aux)
{
        static const uint16_t freqs[279] = {
        /*   0 */ 0,
        /*   1 */ 0,
        /*   2 */ 0,
        /*   3 */ 0,
        /*   4 */ 0,
        /*   5 */ 0,
        /*   6 */ 0,
        /*   7 */ 0,
        /*   8 */ 0,
        /*   9 */ 0,
        /*  10 */ 48,
        /*  11 */ 0,
        /*  12 */ 1,
        /*  13 */ 0,
        /*  14 */ 0,
        /*  15 */ 0,
        /*  16 */ 0,
        /*  17 */ 0,
        /*  18 */ 0,
        /*  19 */ 0,
        /*  20 */ 0,
        /*  21 */ 0,
        /*  22 */ 0,
        /*  23 */ 0,
        /*  24 */ 0,
        /*  25 */ 0,
        /*  26 */ 0,
        /*  27 */ 0,
        /*  28 */ 0,
        /*  29 */ 0,
        /*  30 */ 0,
        /*  31 */ 0,
        /*  32 */ 158,
        /*  33 */ 1,
        /*  34 */ 16,
        /*  35 */ 1,
        /*  36 */ 0,
        /*  37 */ 1,
        /*  38 */ 0,
        /*  39 */ 5,
        /*  40 */ 43,
        /*  41 */ 41,
        /*  42 */ 6,
        /*  43 */ 14,
        /*  44 */ 68,
        /*  45 */ 64,
        /*  46 */ 73,
        /*  47 */ 19,
        /*  48 */ 50,
        /*  49 */ 92,
        /*  50 */ 61,
        /*  51 */ 67,
        /*  52 */ 37,
        /*  53 */ 51,
        /*  54 */ 37,
        /*  55 */ 36,
        /*  56 */ 38,
        /*  57 */ 33,
        /*  58 */ 22,
        /*  59 */ 20,
        /*  60 */ 11,
        /*  61 */ 10,
        /*  62 */ 6,
        /*  63 */ 0,
        /*  64 */ 2,
        /*  65 */ 28,
        /*  66 */ 11,
        /*  67 */ 17,
        /*  68 */ 11,
        /*  69 */ 20,
        /*  70 */ 11,
        /*  71 */ 9,
        /*  72 */ 6,
        /*  73 */ 14,
        /*  74 */ 3,
        /*  75 */ 5,
        /*  76 */ 22,
        /*  77 */ 14,
        /*  78 */ 15,
        /*  79 */ 2,
        /*  80 */ 14,
        /*  81 */ 1,
        /*  82 */ 9,
        /*  83 */ 23,
        /*  84 */ 13,
        /*  85 */ 6,
        /*  86 */ 4,
        /*  87 */ 6,
        /*  88 */ 6,
        /*  89 */ 4,
        /*  90 */ 5,
        /*  91 */ 4,
        /*  92 */ 2,
        /*  93 */ 11,
        /*  94 */ 1,
        /*  95 */ 3,
        /*  96 */ 0,
        /*  97 */ 116,
        /*  98 */ 37,
        /*  99 */ 56,
        /* 100 */ 65,
        /* 101 */ 166,
        /* 102 */ 36,
        /* 103 */ 41,
        /* 104 */ 36,
        /* 105 */ 119,
        /* 106 */ 2,
        /* 107 */ 10,
        /* 108 */ 79,
        /* 109 */ 56,
        /* 110 */ 107,
        /* 111 */ 108,
        /* 112 */ 54,
        /* 113 */ 2,
        /* 114 */ 100,
        /* 115 */ 140,
        /* 116 */ 101,
        /* 117 */ 80,
        /* 118 */ 21,
        /* 119 */ 42,
        /* 120 */ 15,
        /* 121 */ 31,
        /* 122 */ 11,
        /* 123 */ 1,
        /* 124 */ 7,
        /* 125 */ 2,
        /* 126 */ 0,
        /* 127 */ 0,
        /* 128 */ 0,
        /* 129 */ 0,
        /* 130 */ 0,
        /* 131 */ 0,
        /* 132 */ 0,
        /* 133 */ 0,
        /* 134 */ 0,
        /* 135 */ 0,
        /* 136 */ 0,
        /* 137 */ 0,
        /* 138 */ 0,
        /* 139 */ 0,
        /* 140 */ 0,
        /* 141 */ 0,
        /* 142 */ 0,
        /* 143 */ 0,
        /* 144 */ 0,
        /* 145 */ 0,
        /* 146 */ 0,
        /* 147 */ 0,
        /* 148 */ 0,
        /* 149 */ 0,
        /* 150 */ 0,
        /* 151 */ 0,
        /* 152 */ 0,
        /* 153 */ 0,
        /* 154 */ 0,
        /* 155 */ 0,
        /* 156 */ 0,
        /* 157 */ 0,
        /* 158 */ 0,
        /* 159 */ 0,
        /* 160 */ 0,
        /* 161 */ 0,
        /* 162 */ 0,
        /* 163 */ 0,
        /* 164 */ 0,
        /* 165 */ 0,
        /* 166 */ 0,
        /* 167 */ 0,
        /* 168 */ 0,
        /* 169 */ 0,
        /* 170 */ 0,
        /* 171 */ 0,
        /* 172 */ 0,
        /* 173 */ 0,
        /* 174 */ 0,
        /* 175 */ 0,
        /* 176 */ 0,
        /* 177 */ 0,
        /* 178 */ 0,
        /* 179 */ 0,
        /* 180 */ 0,
        /* 181 */ 0,
        /* 182 */ 0,
        /* 183 */ 0,
        /* 184 */ 0,
        /* 185 */ 0,
        /* 186 */ 0,
        /* 187 */ 0,
        /* 188 */ 0,
        /* 189 */ 0,
        /* 190 */ 0,
        /* 191 */ 0,
        /* 192 */ 0,
        /* 193 */ 0,
        /* 194 */ 0,
        /* 195 */ 0,
        /* 196 */ 0,
        /* 197 */ 0,
        /* 198 */ 0,
        /* 199 */ 0,
        /* 200 */ 0,
        /* 201 */ 0,
        /* 202 */ 0,
        /* 203 */ 0,
        /* 204 */ 0,
        /* 205 */ 0,
        /* 206 */ 0,
        /* 207 */ 0,
        /* 208 */ 0,
        /* 209 */ 0,
        /* 210 */ 0,
        /* 211 */ 0,
        /* 212 */ 0,
        /* 213 */ 0,
        /* 214 */ 0,
        /* 215 */ 0,
        /* 216 */ 0,
        /* 217 */ 0,
        /* 218 */ 0,
        /* 219 */ 0,
        /* 220 */ 0,
        /* 221 */ 0,
        /* 222 */ 0,
        /* 223 */ 0,
        /* 224 */ 0,
        /* 225 */ 0,
        /* 226 */ 0,
        /* 227 */ 0,
        /* 228 */ 0,
        /* 229 */ 0,
        /* 230 */ 0,
        /* 231 */ 0,
        /* 232 */ 0,
        /* 233 */ 0,
        /* 234 */ 0,
        /* 235 */ 0,
        /* 236 */ 0,
        /* 237 */ 0,
        /* 238 */ 0,
        /* 239 */ 0,
        /* 240 */ 0,
        /* 241 */ 0,
        /* 242 */ 0,
        /* 243 */ 0,
        /* 244 */ 0,
        /* 245 */ 0,
        /* 246 */ 0,
        /* 247 */ 0,
        /* 248 */ 0,
        /* 249 */ 0,
        /* 250 */ 0,
        /* 251 */ 0,
        /* 252 */ 0,
        /* 253 */ 0,
        /* 254 */ 0,
        /* 255 */ 0,
        /* 256 */ 1,
        /* 257 */ 497,
        /* 258 */ 533,
        /* 259 */ 473,
        /* 260 */ 385,
        /* 261 */ 335,
        /* 262 */ 255,
        /* 263 */ 191,
        /* 264 */ 132,
        /* 265 */ 241,
        /* 266 */ 170,
        /* 267 */ 135,
        /* 268 */ 89,
        /* 269 */ 115,
        /* 270 */ 39,
        /* 271 */ 32,
        /* 272 */ 11,
        /* 273 */ 24,
        /* 274 */ 15,
        /* 275 */ 9,
        /* 276 */ 4,
        /* 277 */ 22,
        /* 278 */ 10,
        };

        huffman_encoder_t e;

        huffman_encoder_init(&e, freqs, sizeof(freqs) / sizeof(freqs[0]), 15);

        (void)aux;
}

void bench_huffman_encoder_init_teardown(void *aux)
{
        (void)aux;
}

size_t bench_huffman_encoder_init_bytes(void)
{
        return 0;
}
