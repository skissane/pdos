/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "bits.h"
#include "bitstream.h"
#include "huffman.h"

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        huffman_decoder_t d;
        istream_t is;
        uint16_t num_symbols;
        uint8_t lens[MAX_HUFFMAN_SYMBOLS];
        size_t i, used;
        int sym;

        if (size < 2) {
                return 0;
        }

        num_symbols = read16le(data);
        if (num_symbols > MAX_HUFFMAN_SYMBOLS) {
                num_symbols = MAX_HUFFMAN_SYMBOLS;
        }
        data += 2;
        size -= 2;

        for (i = 0; i < num_symbols; i++) {
                if (size == 0) {
                        return 0;
                }
                lens[i] = data[0];
                if (lens[i] > MAX_HUFFMAN_BITS) {
                        lens[i] = MAX_HUFFMAN_BITS;
                }

                data++;
                size--;
        }

        if (!huffman_decoder_init(&d, lens, num_symbols)) {
                return 0;
        }

        istream_init(&is, data, size);

        while (true) {
                sym = huffman_decode(&d, (uint16_t)istream_bits(&is), &used);
                if (sym == -1) {
                       return 0;
                }
                assert(sym < num_symbols);
                assert(used >= 1 && used <= MAX_HUFFMAN_BITS);
                if (!istream_advance(&is, used)) {
                        return 0;
                }
        }
}
