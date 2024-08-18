/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "shrink.h"

#include <stdlib.h>
#include <string.h>
#include "bitstream.h"
#include "hamlet.h"
#include "test_utils.h"

void test_shrink_empty(void)
{
        static const uint8_t data[1];
        uint8_t dst[1];
        size_t used = 99;

        /* Empty src. */
        dst[0] = 0x42;
        CHECK(hwshrink(&data[1], 0, dst, sizeof(dst), &used));
        CHECK(used == 0);
        CHECK(dst[0] == 0x42);

        /* Empty src, empty dst. */
        CHECK(hwshrink(&data[1], 0, &dst[1], 0, &used));
        CHECK(used == 0);

        /* Empty dst. */
        CHECK(!hwshrink(data, sizeof(data), &dst[1], 0, &used));
}

/*
   $ curl -O http://cd.textfiles.com/1stcanadian/utils/pkz110/pkz110.exe
   $ unzip pkz110.exe PKZIP.EXE
   $ echo -n ababcbababaaaaaaa > x
   $ dosbox -c "mount c ." -c "c:" -c "pkzip -es x.zip x" -c exit
   $ xxd -i -s 31 -l $(expr $(find X.ZIP -printf %s) - 100) X.ZIP
 */
static const uint8_t lzw_fig5[17] = "ababcbababaaaaaaa";
static const uint8_t lzw_fig5_shrunk[] = {
0x61, 0xc4, 0x04, 0x1c, 0x23, 0xb0, 0x60, 0x98, 0x83, 0x08, 0xc3, 0x00
};

void test_shrink_basic(void)
{
        uint8_t dst[100];
        size_t used;

        CHECK(hwshrink(lzw_fig5, sizeof(lzw_fig5), dst, sizeof(dst), &used));
        CHECK(used == sizeof(lzw_fig5_shrunk));
        CHECK(memcmp(dst, lzw_fig5_shrunk, sizeof(lzw_fig5_shrunk)) == 0);
}

static void roundtrip(const uint8_t *src, size_t src_len)
{
        uint8_t *compressed;
        uint8_t *uncompressed;
        size_t compressed_cap, compressed_size;
        size_t uncompressed_size, used;

        compressed_cap = src_len * 2 + 100;
        compressed = malloc(compressed_cap);
        uncompressed = malloc(src_len);

        CHECK(hwshrink(src, src_len, compressed, compressed_cap,
                       &compressed_size));

        CHECK(hwunshrink(compressed, compressed_size, &used, uncompressed,
                         src_len, &uncompressed_size) == HWUNSHRINK_OK);
        CHECK(used == compressed_size);
        CHECK(uncompressed_size == src_len);
        CHECK(memcmp(uncompressed, src, src_len) == 0);

        free(compressed);
        free(uncompressed);
}

void test_shrink_many_codes(void)
{
        uint8_t src[25 * 256 * 2];
        uint8_t dst[sizeof(src) * 2];
        size_t i, j, src_size, tmp;

        /* This will churn through new codes pretty fast, causing code size
         * increase and partial clearing multiple times. */
        src_size = 0;
        for (i = 0; i < 25; i++) {
                for (j = 0; j < 256; j++) {
                        src[src_size++] = (uint8_t)i;
                        src[src_size++] = (uint8_t)j;
                }
        }

        roundtrip(src, src_size);

        /* Try shrinking into a too small buffer. */

        /* Hit the buffer full case while signaling increased code size. */
        for (i = 0; i < 600; i++) {
                CHECK(!hwshrink(src, src_size, dst, i, &tmp));
        }
        /* Hit the buffer full case while signaling partial clearing. */
        for (i = 11000; i < 12000; i++) {
                CHECK(!hwshrink(src, src_size, dst, i, &tmp));
        }
}

void test_shrink_aaa(void)
{
        static const size_t src_size = 61505 * 1024;
        uint8_t *src;

        /* This adds codes to the table which are all prefixes of each other.
         * Then each partial clearing will create a self-referential code,
         * which means that code is lost. Eventually all codes are lost this
         * way, and the bytes are all encoded as literals. */

        src = malloc(src_size);
        memset(src, 'a', src_size);
        roundtrip(src, src_size);
        free(src);
}

void test_shrink_hamlet(void)
{
        uint8_t compressed[100 * 1024], uncompressed[sizeof(hamlet)];
        size_t compressed_size, used, uncompressed_size;

        CHECK(hwshrink(hamlet, sizeof(hamlet),
                       compressed, sizeof(compressed), &compressed_size));

        /* Update if we make compression better. */
        CHECK(compressed_size == 93900);

        /* PKZIP 1.10
           pkzip -es a.zip hamlet.txt
           93900 bytes */

        CHECK(hwunshrink(compressed, compressed_size, &used, uncompressed,
                         sizeof(hamlet), &uncompressed_size) == HWUNSHRINK_OK);
        CHECK(used == compressed_size);
        CHECK(uncompressed_size == sizeof(hamlet));
        CHECK(memcmp(uncompressed, hamlet, sizeof(hamlet)) == 0);
}

void test_unshrink_empty(void)
{
        static const uint8_t data[2];
        uint8_t dst[1];
        size_t src_used = 123;
        size_t dst_used = 456;

        /* Empty src. */
        dst[0] = 0x42;
        CHECK(hwunshrink(&data[2], 0, &src_used, dst, sizeof(dst), &dst_used)
                        == HWUNSHRINK_OK);
        CHECK(src_used == 0);
        CHECK(dst_used == 0);
        CHECK(dst[0] == 0x42);

        /* Empty src, empty dst. */
        CHECK(hwunshrink(&data[2], 0, &src_used, &dst[1], 0, &dst_used)
                        == HWUNSHRINK_OK);
        CHECK(src_used == 0);
        CHECK(dst_used == 0);

        /* Empty dst. */
        CHECK(hwunshrink(data, sizeof(data), &src_used, &dst[1], 0, &dst_used)
                        == HWUNSHRINK_FULL);
}

void test_unshrink_basic(void)
{
        uint8_t comp[100], decomp[100];
        size_t comp_size, comp_used, decomp_used;
        ostream_t os;

        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9);
        ostream_write(&os, 'b', 9); /* New code: 257 = "ab" */
        ostream_write(&os, 'c', 9); /* New code: 258 = "bc" */
        ostream_write(&os, 257, 9); /* New code: 259 = "ca" */
        ostream_write(&os, 259, 9);
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_OK);
        CHECK(comp_used == comp_size);
        CHECK(decomp_used == 7);
        CHECK(decomp[0] == 'a');
        CHECK(decomp[1] == 'b');
        CHECK(decomp[2] == 'c');
        CHECK(decomp[3] == 'a'); /* 257 */
        CHECK(decomp[4] == 'b');
        CHECK(decomp[5] == 'c'); /* 259 */
        CHECK(decomp[6] == 'a');

        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9);
        ostream_write(&os, 'b', 9); /* New code: 257 = "ab" */
        ostream_write(&os, 456, 9); /* Invalid code! */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_ERR);
}

void test_unshrink_snag(void)
{
        /* Hit the "KwKwKw" case, or LZW snag, where the decompressor sees the
           next code before it's been added to the table. */

        uint8_t comp[100], decomp[100];
        size_t comp_size, comp_used, decomp_used;
        ostream_t os;

        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9); /* Emit "a" */
        ostream_write(&os, 'n', 9); /* Emit "n";  new code: 257 = "an" */
        ostream_write(&os, 'b', 9); /* Emit "b";  new code: 258 = "nb" */
        ostream_write(&os, 257, 9); /* Emit "an"; new code: 259 = "ba" */

        ostream_write(&os, 260, 9); /* The LZW snag
                                       Emit and add 260 = "ana" */

        comp_size = ostream_bytes_written(&os);

        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_OK);
        CHECK(comp_used == comp_size);

        CHECK(decomp_used == 8);
        CHECK(decomp[0] == 'a');
        CHECK(decomp[1] == 'n');
        CHECK(decomp[2] == 'b');
        CHECK(decomp[3] == 'a'); /* 257 */
        CHECK(decomp[4] == 'n');
        CHECK(decomp[5] == 'a'); /* 260 */
        CHECK(decomp[6] == 'n');
        CHECK(decomp[7] == 'a');

        /* Test hitting the LZW snag where the previous code is invalid. */
        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9); /* Emit "a" */
        ostream_write(&os, 'b', 9); /* Emit "b";  new code: 257 = "ab" */
        ostream_write(&os, 'c', 9); /* Emit "c";  new code: 258 = "bc" */
        ostream_write(&os, 258, 9); /* Emit "bc"; new code: 259 = "cb" */
        ostream_write(&os, 256, 9); /* Partial clear, dropping codes: */
        ostream_write(&os,   2, 9); /*                257, 258, 259 */
        ostream_write(&os, 257, 9); /* LZW snag; previous code is invalid. */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_ERR);
}

void test_unshrink_early_snag(void)
{
        /* Hit the LZW snag right at the start. Not sure if this can really
           happen, but handle it anyway. */

        uint8_t comp[100], decomp[100];
        size_t comp_size, comp_used, decomp_used;
        ostream_t os;

        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9); /* Emit "a" */
        ostream_write(&os, 257, 9); /* The LZW snag
                                       Emit and add 257 = "aa" */

        comp_size = ostream_bytes_written(&os);

        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_OK);
        CHECK(comp_used == comp_size);

        CHECK(decomp_used == 3);
        CHECK(decomp[0] == 'a');
        CHECK(decomp[1] == 'a');
        CHECK(decomp[2] == 'a'); /* 257 */
}

void test_unshrink_tricky_first_code(void)
{

        uint8_t comp[100], decomp[100];
        size_t comp_size, comp_used, decomp_used;
        ostream_t os;

        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 257, 9); /* An unused code. */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_ERR);

        /* Handle control codes also for the first code. (Not sure if PKZIP
           can handle that, but we do.) */

        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 256,  9); /* Control code. */
        ostream_write(&os,   1,  9); /* Code size increase. */
        ostream_write(&os, 'a', 10); /* 'a'. */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_OK);
        CHECK(decomp_used == 1);
        CHECK(decomp[0] == 'a');

        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 256, 9); /* Control code. */
        ostream_write(&os,   2, 9); /* Partial clear. */
        ostream_write(&os, 'a', 9); /* 'a'. */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_OK);
        CHECK(decomp_used == 1);
        CHECK(decomp[0] == 'a');
}

void test_unshrink_invalidated_prefix_codes(void)
{
        uint8_t comp[100], decomp[100];
        size_t comp_size, comp_used, decomp_used;
        ostream_t os;

        /* Code where the prefix code hasn't been re-used. */
        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9); /* Emit "a" */
        ostream_write(&os, 'b', 9); /* Emit "b";  new code: 257 = "ab" */
        ostream_write(&os, 'c', 9); /* Emit "c";  new code: 258 = "bc" */
        ostream_write(&os, 'd', 9); /* Emit "d";  new code: 259 = "cd" */
        ostream_write(&os, 259, 9); /* Emit "cd"; new code: 260 = "dc" */
        ostream_write(&os, 256, 9); /* Partial clear, dropping codes: */
        ostream_write(&os,   2, 9); /*     257, 258, 259, 260 */
        ostream_write(&os, 'x', 9); /* Emit "x"; new code: 257 = {259}+"x" */
        ostream_write(&os, 257, 9); /* Error: 257's prefix is invalid. */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_ERR);

        /* Code there the prefix code has been re-used. */
        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9); /* Emit "a" */
        ostream_write(&os, 'b', 9); /* Emit "b";  new code: 257 = "ab" */
        ostream_write(&os, 'c', 9); /* Emit "c";  new code: 258 = "bc" */
        ostream_write(&os, 'd', 9); /* Emit "d";  new code: 259 = "cd" */
        ostream_write(&os, 259, 9); /* Emit "cd"; new code: 260 = "dc" */
        ostream_write(&os, 256, 9); /* Partial clear, dropping codes: */
        ostream_write(&os,   2, 9); /*     257, 258, 259, 260 */
        ostream_write(&os, 'x', 9); /* Emit "x";  new code: 257 = {259}+"x" */
        ostream_write(&os, 'y', 9); /* Emit "y";  new code: 258 = "xy" */
        ostream_write(&os, 'z', 9); /* Emit "z";  new code: 259 = "yz" */
        ostream_write(&os, '0', 9); /* Emit "0";  new code: 260 = "z0" */
        ostream_write(&os, 257, 9); /* Emit "yzx" */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_OK);
        CHECK(comp_used == comp_size);
        CHECK(decomp_used == 13);
        CHECK(decomp[0]  == 'a');
        CHECK(decomp[1]  == 'b');
        CHECK(decomp[2]  == 'c');
        CHECK(decomp[3]  == 'd');
        CHECK(decomp[4]  == 'c');
        CHECK(decomp[5]  == 'd');
        CHECK(decomp[6]  == 'x');
        CHECK(decomp[7]  == 'y');
        CHECK(decomp[8]  == 'z');
        CHECK(decomp[9]  == '0');
        CHECK(decomp[10] == 'y');
        CHECK(decomp[11] == 'z');
        CHECK(decomp[12] == 'x');

        /* Code where the prefix gets re-used by the next code (i.e. the LZW
         * snag). This is the trickiest case. */
        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9); /* Emit "a" */
        ostream_write(&os, 'b', 9); /* Emit "b";  new code: 257 = "ab" */
        ostream_write(&os, 'c', 9); /* Emit "c";  new code: 258 = "bc" */
        ostream_write(&os, 'd', 9); /* Emit "d";  new code: 259 = "cd" */
        ostream_write(&os, 'e', 9); /* Emit "e";  new code: 260 = "de" */
        ostream_write(&os, 'f', 9); /* Emit "f";  new code: 261 = "ef" */
        ostream_write(&os, 261, 9); /* Emit "ef"; new code: 262 = "fe" */
        ostream_write(&os, 256, 9); /* Partial clear, dropping codes: */
        ostream_write(&os,   2, 9); /*     257, 258, 259, 260, 261, 262 */

        ostream_write(&os, 'a', 9); /* Emit "a";  new code: 257={261}+"a" */
        ostream_write(&os, 'n', 9); /* Emit "n";  new code: 258 = "an" */
        ostream_write(&os, 'b', 9); /* Emit "b";  new code: 259 = "nb" */
        ostream_write(&os, 258, 9); /* Emit "an"; new code: 260 = "ba" */
        ostream_write(&os, 257, 9); /* Emit "anaa". (new old code 261="ana") */

        /* Just to be sure 261 and 257 are represented correctly now: */
        ostream_write(&os, 261, 9); /* Emit "ana"; new code 262="aana" */
        ostream_write(&os, 257, 9); /* Emit "anaa"; new code 263="aanaa" */

        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_OK);
        CHECK(comp_used == comp_size);
        CHECK(decomp_used == 24);
        CHECK(decomp[0]  == 'a');
        CHECK(decomp[1]  == 'b');
        CHECK(decomp[2]  == 'c');
        CHECK(decomp[3]  == 'd');
        CHECK(decomp[4]  == 'e');
        CHECK(decomp[5]  == 'f');
        CHECK(decomp[6]  == 'e');
        CHECK(decomp[7]  == 'f');
        CHECK(decomp[8]  == 'a');
        CHECK(decomp[9]  == 'n');
        CHECK(decomp[10] == 'b');
        CHECK(decomp[11] == 'a');
        CHECK(decomp[12] == 'n');
        CHECK(decomp[13] == 'a');
        CHECK(decomp[14] == 'n');
        CHECK(decomp[15] == 'a');
        CHECK(decomp[16] == 'a');

        CHECK(decomp[17] == 'a');
        CHECK(decomp[18] == 'n');
        CHECK(decomp[19] == 'a');
        CHECK(decomp[20] == 'a');
        CHECK(decomp[21] == 'n');
        CHECK(decomp[22] == 'a');
        CHECK(decomp[23] == 'a');
}

void test_unshrink_self_prefix(void)
{

        uint8_t comp[100], decomp[100];
        size_t comp_size, comp_used, decomp_used;
        ostream_t os;

        /* Create self-prefixed code and try to use it. */
        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9); /* Emit "a" */
        ostream_write(&os, 'b', 9); /* Emit "b";  new code: 257 = "ab" */
        ostream_write(&os, 257, 9); /* Emit "ab"; new code: 258 = "ba" */
        ostream_write(&os, 256, 9); /* Partial clear, dropping codes: */
        ostream_write(&os,   2, 9); /*     257, 258 */
        ostream_write(&os, 'a', 9); /* Emit "a"; new code: 257 = {257}+"a" */
        ostream_write(&os, 257, 9); /* Error: 257 cannot be used. */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_ERR);


        /* Create self-prefixed code and check that it's not recycled by
         * partial clearing. */
        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9); /* Emit "a" */
        ostream_write(&os, 'b', 9); /* Emit "b";  new code: 257 = "ab" */
        ostream_write(&os, 257, 9); /* Emit "ab"; new code: 258 = "ba" */
        ostream_write(&os, 256, 9); /* Partial clear, dropping codes: */
        ostream_write(&os,   2, 9); /*     257, 258 */
        ostream_write(&os, 'a', 9); /* Emit "a"; new code: 257 = {257}+"a" */
        ostream_write(&os, 'b', 9); /* Emit "b"; new code: 258 = "ab" */
        ostream_write(&os, 256, 9); /* Partial clear, dropping codes: */
        ostream_write(&os,   2, 9); /* 258 (Note that 257 isn't re-used) */
        ostream_write(&os, 'x', 9); /* Emit "x"; new code: 258 = "bx" */
        ostream_write(&os, 258, 9); /* Emit "bx". */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_OK);
        CHECK(comp_used == comp_size);
        CHECK(decomp_used == 9);
        CHECK(decomp[0]  == 'a');
        CHECK(decomp[1]  == 'b');
        CHECK(decomp[2]  == 'a');
        CHECK(decomp[3]  == 'b');
        CHECK(decomp[4]  == 'a');
        CHECK(decomp[5]  == 'b');
        CHECK(decomp[6]  == 'x');
        CHECK(decomp[7]  == 'b');
        CHECK(decomp[8]  == 'x');
}

void test_unshrink_too_short(void)
{
        /* Test with too short src and dst. */
        uint8_t comp[100], decomp[100];
        size_t comp_size, comp_used, decomp_used;
        ostream_t os;
        size_t i;
        unshrnk_stat_t s;

        /* Code where the prefix gets re-used by the next code (i.e. the LZW
         * snag). Copied from test_unshrink_invalidated_prefix_codes. */
        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9); /* Emit "a" */
        ostream_write(&os, 'b', 9); /* Emit "b";  new code: 257 = "ab" */
        ostream_write(&os, 'c', 9); /* Emit "c";  new code: 258 = "bc" */
        ostream_write(&os, 'd', 9); /* Emit "d";  new code: 259 = "cd" */
        ostream_write(&os, 'e', 9); /* Emit "e";  new code: 260 = "de" */
        ostream_write(&os, 'f', 9); /* Emit "f";  new code: 261 = "ef" */
        ostream_write(&os, 261, 9); /* Emit "ef"; new code: 262 = "fe" */
        ostream_write(&os, 256, 9); /* Partial clear, dropping codes: */
        ostream_write(&os,   2, 9); /*     257, 258, 259, 260, 261, 262 */

        ostream_write(&os, 'a', 9); /* Emit "a";  new code: 257={261}+"a" */
        ostream_write(&os, 'n', 9); /* Emit "n";  new code: 258 = "an" */
        ostream_write(&os, 'b', 9); /* Emit "b";  new code: 259 = "nb" */
        ostream_write(&os, 258, 9); /* Emit "an"; new code: 260 = "ba" */
        ostream_write(&os, 257, 9); /* Emit "anaa". (new old code 261="ana") */

        /* Just to be sure 261 and 257 are represented correctly now: */
        ostream_write(&os, 261, 9); /* Emit "ana"; new code 262="aana" */
        ostream_write(&os, 257, 9); /* Emit "anaa"; new code 263="aanaa" */

        comp_size = ostream_bytes_written(&os);

        /* This is the expected full output. */
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_OK);
        CHECK(comp_used == comp_size);
        CHECK(decomp_used == 24);
        CHECK(memcmp(decomp, "abcdefefanbananaaanaanaa", 24) == 0);

        /* Test not enough input bytes. It should error or output something
           shorter. */
        for (i = 0; i < comp_size; i++) {
                s = hwunshrink(comp, i, &comp_used, decomp, sizeof(decomp),
                               &decomp_used);
                if (s == HWUNSHRINK_OK) {
                        CHECK(comp_used <= i);
                        CHECK(decomp_used < 24);
                        CHECK(memcmp(decomp, "abcdefefanbananaaanaanaa",
                                     decomp_used) == 0);
                } else {
                        CHECK(s == HWUNSHRINK_ERR);
                }
        }

        /* Test not having enough room for the output. */
        for (i = 0; i < 24; i++) {
               decomp[i] = 0x42;
                s = hwunshrink(comp, comp_size, &comp_used, decomp, i,
                               &decomp_used);
                CHECK(s == HWUNSHRINK_FULL);
                CHECK(decomp[i] == 0x42);
        }
}

void test_unshrink_bad_control_code(void)
{
        uint8_t comp[100], decomp[100];
        size_t comp_size, comp_used, decomp_used;
        ostream_t os;
        size_t i, codesize;

        /* Only 1 and 2 are valid control code values. */
        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', 9); /* Emit "a" */
        ostream_write(&os, 256, 9);
        ostream_write(&os,   3, 9); /* Invalid control code. */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_ERR);

        /* Try increasing the code size too much. */
        codesize = 9;
        ostream_init(&os, comp, sizeof(comp));
        ostream_write(&os, 'a', codesize); /* Emit "a" */
        for (i = 0; i < 10; i++) {
                ostream_write(&os, 256, codesize);
                ostream_write(&os,   1, codesize); /* Increase code size. */
                codesize++;
        }
        ostream_write(&os, 'b', codesize); /* Emit "b" */
        comp_size = ostream_bytes_written(&os);
        CHECK(hwunshrink(comp, comp_size, &comp_used, decomp, sizeof(decomp),
                         &decomp_used) == HWUNSHRINK_ERR);
}

void test_unshrink_lzw_fig5(void)
{
        uint8_t dst[100];
        size_t src_used, dst_used;

        CHECK(hwunshrink(lzw_fig5_shrunk, sizeof(lzw_fig5_shrunk), &src_used,
                         dst, sizeof(dst), &dst_used) == HWUNSHRINK_OK);
        CHECK(src_used == sizeof(lzw_fig5_shrunk));
        CHECK(dst_used == sizeof(lzw_fig5));
        CHECK(memcmp(dst, lzw_fig5, sizeof(lzw_fig5)) == 0);
}
