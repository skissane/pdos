/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "implode.h"

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        uint8_t compressed[2 * 1024 * 1024];
        uint8_t decompressed[1024 * 1024];
        size_t compressed_sz, compressed_used;
        bool large_wnd, lit_tree;

        if (size > sizeof(decompressed)) {
                return 0;
        }

        if (size < 1) {
                return 0;
        }

        large_wnd = (data[0] >> 0) & 1;
        lit_tree  = (data[0] >> 1) & 1;
        size--;
        data++;

        /* Check that we can compress it. */
        assert(hwimplode(data, size, large_wnd, lit_tree,
                         compressed, sizeof(compressed), &compressed_sz));

        /* Check that we can decompress it. */
        assert(hwexplode(compressed, compressed_sz,
                         size, large_wnd, lit_tree, false,
                         &compressed_used, decompressed) == HWEXPLODE_OK);
        assert(compressed_used == compressed_sz);
        assert(memcmp(decompressed, data, size) == 0);

        return 0;
}
