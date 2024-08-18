/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "implode.h"

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        uint8_t decompressed[1024 * 1024];
        size_t compressed_used;
        bool large_wnd, lit_tree, bug_compat;

        if (size < 1) {
                return 0;
        }

        large_wnd  = (data[0] >> 0) & 1;
        lit_tree   = (data[0] >> 1) & 1;
        bug_compat = (data[0] >> 2) & 1;
        data++;
        size--;

        /* Check that we don't assert or hit ASan problems. */

        hwexplode(data, size, sizeof(decompressed),
                  large_wnd, lit_tree, bug_compat,
                  &compressed_used, decompressed);

        return 0;
}
