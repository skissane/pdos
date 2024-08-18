/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "crc32.h"
#include "deflate.h"
#include "zip.h"

#ifdef NDEBUG
#error "Asserts must be enabled!"
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
        zip_t z;
        size_t n;
        zipiter_t it;
        zipmemb_t m;
        uint8_t *extracted;

        if (!zip_read(&z, data, size)) {
                return 0;
        }

        n = 0;
        for (it = z.members_begin; it != z.members_end; it = m.next) {
                n++;
                m = zip_member(&z, it);

                assert(m.name + m.name_len <= data + size);
                assert(m.comment + m.comment_len <= data + size);
                assert(m.comp_data + m.comp_size <= data + size);

                extracted = malloc(m.uncomp_size);
                if (extracted == NULL) {
                        continue;
                }

                if (zip_extract_member(&m, extracted)) {
                        crc32(extracted, m.uncomp_size);
                }
                free(extracted);
        }
        assert(n == z.num_members);

        return 0;
}
