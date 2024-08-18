/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "bits.h"
#include "huffman.h"

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        uint16_t num_symbols;
        uint16_t freq, freqs[MAX_HUFFMAN_SYMBOLS];
        size_t freq_sum, i;
        huffman_encoder_t enc;

        num_symbols = (uint16_t)(size / 2);
        if (num_symbols > MAX_HUFFMAN_SYMBOLS) {
                num_symbols = MAX_HUFFMAN_SYMBOLS;
        }

        freq_sum = 0;
        for (i = 0; i < num_symbols; i++) {
                freq = read16le(&data[2 * i]);
                freq_sum += freq;

                if (freq_sum <= UINT16_MAX) {
                        freqs[i] = freq;
                } else {
                        freqs[i] = 0;
                }
        }

        huffman_encoder_init(&enc, freqs, num_symbols, 15);

        return 0;
}
