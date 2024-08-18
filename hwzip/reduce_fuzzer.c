/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

/* It can be useful to fuzz with reduced MAX_CODESIZE, to hit more interesting
   code paths quicker. */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "reduce.h"

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        uint8_t compressed[2 * 1024 * 1024];
        uint8_t decompressed[1024 * 1024];
        size_t compressed_sz, compressed_used;
        int comp_factor;

        if (size > sizeof(decompressed)) {
                return 0;
        }

        if (size < 1) {
                return 0;
        }

        comp_factor = (data[0] % 4) + 1;
        size--;
        data++;

        /* Check that we can compress it. */
        assert(hwreduce(data, size, comp_factor, compressed,
                        sizeof(compressed), &compressed_sz));

        /* Check that we can decompress it. */
        assert(hwexpand(compressed, compressed_sz, size, comp_factor,
                        &compressed_used, decompressed) == HWEXPAND_OK);
        assert(compressed_used == compressed_sz);
        assert(memcmp(decompressed, data, size) == 0);

        return 0;
}
