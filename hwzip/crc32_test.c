/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "crc32.h"

#include "test_utils.h"

void test_crc32_basic(void)
{
        /* "Check" value from https://www.zlib.net/crc_v3.txt */
        CHECK(crc32((const uint8_t*)"123456789", 9) == 0xcbf43926);

        /* $ crc32 /dev/null */
        CHECK(crc32((const uint8_t*)"", 0) == 0x00000000);

        /* $ crc32 <(echo -n x) */
        CHECK(crc32((const uint8_t*)"x", 1) == 0x8cdc1683);

        /* $ crc32 <(echo -n hwzip is great) */
        CHECK(crc32((const uint8_t*)"hwzip is great", 14) == 0xa2da0545);
}
