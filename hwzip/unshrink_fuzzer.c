/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "shrink.h"

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        uint8_t decompressed[1024 * 1024];
        size_t compressed_used, decompressed_sz;

        /* Check that we don't assert or hit ASan problems. */
        hwunshrink(data, size, &compressed_used, decompressed,
                   sizeof(decompressed), &decompressed_sz);

        return 0;
}
