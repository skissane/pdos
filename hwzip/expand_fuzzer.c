/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "reduce.h"

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        uint8_t decompressed[1024 * 1024];
        size_t compressed_used;
        int comp_factor;

        comp_factor = size >= 1 ? ((data[0] % 4) + 1) : 1;

        /* Check that we don't assert or hit ASan problems. */
        hwexpand(data, size, sizeof(decompressed), comp_factor,
                 &compressed_used, decompressed);

        return 0;
}
