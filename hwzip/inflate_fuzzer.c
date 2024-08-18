/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <stddef.h>
#include <stdint.h>
#include "deflate.h"

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        uint8_t uncomp[1024 * 1024];
        size_t src_used, uncomp_sz;

        /* Check that we don't hit any asserts, ASan errors, etc. */
        hwinflate(data, size, &src_used, uncomp, sizeof(uncomp), &uncomp_sz);

        return 0;
}
