/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "lz77.h"

#include <assert.h>
#include <string.h>
#include "hamlet.h"
#include "test_utils.h"

#define MAX_BLOCK_CAP 50000

typedef struct {
        size_t n;
        size_t cap;
        struct {
                size_t dist;
                size_t litlen;
        } data[MAX_BLOCK_CAP];
} block_t;

static bool output_backref(size_t dist, size_t len, void *aux)
{
        block_t *block = aux;

        assert(block->cap <= MAX_BLOCK_CAP);
        assert((dist == 0 && len <= UINT8_MAX) || (dist > 0 && len > 0));
        assert(dist <= LZ_WND_SIZE);

        assert(block->n <= block->cap);
        if (block->n == block->cap) {
                return false;
        }

        block->data[block->n  ].dist = dist;
        block->data[block->n++].litlen = len;

        return true;
}

static bool output_lit(uint8_t lit, void *aux)
{
        return output_backref(0, (size_t)lit, aux);
}

static void unpack(uint8_t *dst, const block_t *b)
{
        size_t i, dst_pos;

        dst_pos = 0;

        for (i = 0; i < b->n; i++) {
                if (b->data[i].dist == 0) {
                        lz77_output_lit(dst, dst_pos,
                                        (uint8_t)b->data[i].litlen);
                        dst_pos++;
                } else {
                        lz77_output_backref(dst, dst_pos, b->data[i].dist,
                                            b->data[i].litlen);
                        dst_pos += b->data[i].litlen;
                }
        }
}


void test_lz77_empty(void)
{
        CHECK(lz77_compress(NULL, 0, 100, 100, true, NULL, NULL, NULL));
}

void test_lz77_literals(void)
{
        static const uint8_t src[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        size_t i, j;

        block_t b;
        b.cap = MAX_BLOCK_CAP;

        for (i = 0; i < sizeof(src); i++) {
                /* Test compressing the i-length prefix of src. */
                b.n = 0;
                CHECK(lz77_compress(src, i, 100, 100, true,
                                    &output_lit, &output_backref, &b));
                CHECK(b.n == i);
                for (j = 0; j < i; j++) {
                        CHECK(b.data[j].dist == 0);
                        CHECK(b.data[j].litlen == src[j]);
                }
        }
}

void test_lz77_backref(void)
{
        static const uint8_t src[] = { 0, 1, 2, 3, 0, 1, 2, 3 };
        block_t b;

        b.cap = MAX_BLOCK_CAP;
        b.n = 0;
        CHECK(lz77_compress(src, sizeof(src), 100, 100, true,
                            &output_lit, &output_backref, &b));
        CHECK(b.n == 5);
        CHECK(b.data[0].dist == 0 && b.data[0].litlen == 0);
        CHECK(b.data[1].dist == 0 && b.data[1].litlen == 1);
        CHECK(b.data[2].dist == 0 && b.data[2].litlen == 2);
        CHECK(b.data[3].dist == 0 && b.data[3].litlen == 3);
        CHECK(b.data[4].dist == 4 && b.data[4].litlen == 4); /* 0, 1, 2, 3 */
}

void test_lz77_aaa(void)
{
        /* An x followed by 300 a's" */
        static const uint8_t s[] = "xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                   "";
        uint8_t out[301];
        block_t b;

        b.cap = MAX_BLOCK_CAP;
        b.n = 0;
        CHECK(lz77_compress(s, 301, 32768, 258, true,
                            &output_lit, &output_backref, &b));
        CHECK(b.n == 4);
        CHECK(b.data[0].dist == 0 && b.data[0].litlen == 'x');
        CHECK(b.data[1].dist == 0 && b.data[1].litlen == 'a');
        CHECK(b.data[2].dist == 1 && b.data[2].litlen == 258);
        CHECK(b.data[3].dist == 1 && b.data[3].litlen == 41);

        unpack(out, &b);
        CHECK(memcmp(s, out, 301) == 0);
}

void test_lz77_remaining_backref(void)
{
        static const uint8_t s[8] = "abcdabcd";
        block_t b;

        b.cap = MAX_BLOCK_CAP;
        b.n = 0;
        CHECK(lz77_compress(s, sizeof(s), 100, 100, true,
                            &output_lit, &output_backref, &b));
        CHECK(b.n == 5);
        CHECK(b.data[0].dist == 0 && b.data[0].litlen == 'a');
        CHECK(b.data[1].dist == 0 && b.data[1].litlen == 'b');
        CHECK(b.data[2].dist == 0 && b.data[2].litlen == 'c');
        CHECK(b.data[3].dist == 0 && b.data[3].litlen == 'd');
        CHECK(b.data[4].dist == 4 && b.data[4].litlen == 4); /* "abcd" */
}

void test_lz77_deferred(void)
{
        static const uint8_t s[] = "x"
                                   "abcde"
                                   "bcdefg"
                                   "abcdefg";

        block_t b;

        b.cap = MAX_BLOCK_CAP;
        b.n = 0;
        CHECK(lz77_compress(s, sizeof(s) - 1, 100, 100, true,
                            &output_lit, &output_backref, &b));

        CHECK(b.n == 11);
        CHECK(b.data[0].dist == 0 && b.data[0].litlen == 'x');

        CHECK(b.data[1].dist == 0 && b.data[1].litlen == 'a');
        CHECK(b.data[2].dist == 0 && b.data[2].litlen == 'b');
        CHECK(b.data[3].dist == 0 && b.data[3].litlen == 'c');
        CHECK(b.data[4].dist == 0 && b.data[4].litlen == 'd');
        CHECK(b.data[5].dist == 0 && b.data[5].litlen == 'e');

        CHECK(b.data[6].dist == 4 && b.data[6].litlen == 4); /* bcde */
        CHECK(b.data[7].dist == 0 && b.data[7].litlen == 'f');
        CHECK(b.data[8].dist == 0 && b.data[8].litlen == 'g');

        /* Could match "abcd" here, but taking a literal "a" and then matching
           "bcdefg" is preferred. */
        CHECK(b.data[9].dist == 0 && b.data[9].litlen == 'a');

        CHECK(b.data[10].dist == 7 && b.data[10].litlen == 6); /* bcdefg */
}

void test_lz77_chain(void)
{
        static const uint8_t s[] = "hippo"
                                   "hippie"
                                   "hippos";

        block_t b;

        b.cap = MAX_BLOCK_CAP;
        b.n = 0;
        CHECK(lz77_compress(s, sizeof(s) - 1, 100, 100, true,
                            &output_lit, &output_backref, &b));

        CHECK(b.n == 10);

        CHECK(b.data[0].dist == 0 && b.data[0].litlen == 'h');
        CHECK(b.data[1].dist == 0 && b.data[1].litlen == 'i');
        CHECK(b.data[2].dist == 0 && b.data[2].litlen == 'p');
        CHECK(b.data[3].dist == 0 && b.data[3].litlen == 'p');
        CHECK(b.data[4].dist == 0 && b.data[4].litlen == 'o');

        CHECK(b.data[5].dist == 5 && b.data[5].litlen == 4); /* hipp */
        CHECK(b.data[6].dist == 0 && b.data[6].litlen == 'i');
        CHECK(b.data[7].dist == 0 && b.data[7].litlen == 'e');

        /* Don't go for "hipp"; look further back the chain. */
        CHECK(b.data[8].dist == 11 && b.data[8].litlen == 5); /* hippo */
        CHECK(b.data[9].dist == 0  && b.data[9].litlen == 's');
}

void test_lz77_output_fail(void)
{
        static const uint8_t s[] = "abcdbcde";
        static const uint8_t t[] = "x123234512345";
        static const uint8_t u[] = "0123123";
        static const uint8_t v[] = "0123";

        block_t b;

        /* Not even room for a literal. */
        b.cap = 0;
        b.n = 0;
        CHECK(!lz77_compress(s, sizeof(s) - 1, 100, 100, true,
                             &output_lit, &output_backref, &b));
        CHECK(b.n == 0);

        /* No room for the backref. */
        b.cap = 4;
        b.n = 0;
        CHECK(!lz77_compress(s, sizeof(s) - 1, 100, 100, true,
                             &output_lit, &output_backref, &b));
        CHECK(b.n == 4); /* a, b, c, d (no room: bcd, e) */

        /* No room for literal for deferred match. */
        b.cap = 8;
        b.n = 0;
        CHECK(!lz77_compress(t, sizeof(t) - 1, 100, 100, true,
                             &output_lit, &output_backref, &b));
        CHECK(b.n == 8); /* x, 1, 2, 3, 2, 3, 4, 5 (no room: 1, 2345) */

        /* No room for final backref. */
        b.cap = 4;
        b.n = 0;
        CHECK(!lz77_compress(u, sizeof(u) - 1, 100, 100, true,
                             &output_lit, &output_backref, &b));
        CHECK(b.n == 4); /* 0, 1, 2, 3 (no room: 1, 2, 3) */

        /* No room for final lit. */
        b.cap = 3;
        b.n = 0;
        CHECK(!lz77_compress(v, sizeof(v) - 1, 100, 100, true,
                             &output_lit, &output_backref, &b));
        CHECK(b.n == 3); /* 0, 1, 2 (no room: 3) */
}

void test_lz77_outside_window(void)
{
        uint8_t s[50000] = {0};
        block_t b;
        size_t i;

        strcpy((char*)&s[1], "foo");
        strcpy((char*)&s[40000], "foo");

        b.cap = MAX_BLOCK_CAP;
        b.n = 0;
        CHECK(lz77_compress(s, sizeof(s), 32768, 100, true,
                            &output_lit, &output_backref, &b));

        CHECK(b.data[1].dist == 0 && b.data[1].litlen == 'f');
        CHECK(b.data[2].dist == 0 && b.data[2].litlen == 'o');
        CHECK(b.data[3].dist == 0 && b.data[3].litlen == 'o');

        /* Search for the next "foo". It can't be a backref, because it's
           outside the window. */
        for (i = 4; i < sizeof(s) - 3; i++) {
                if (b.data[i].dist == 0 && b.data[i].litlen == 'f') {
                        CHECK(b.data[i + 1].dist == 0);
                        CHECK(b.data[i + 1].litlen == 'o');
                        CHECK(b.data[i + 2].dist == 0);
                        CHECK(b.data[i + 2].litlen == 'o');
                        return;
                }
        }
        CHECK(false && "Didn't find the second 'foo'");
}

void test_lz77_hamlet(void)
{
        uint8_t out[sizeof(hamlet)];
        block_t b;

        b.cap = MAX_BLOCK_CAP;
        b.n = 0;
        CHECK(lz77_compress(hamlet, sizeof(hamlet), 32768, 258, true,
                            &output_lit, &output_backref, &b));

        /* Lower this expectation in case of improvements to the algorithm. */
        CHECK(b.n == 47990);

        unpack(out, &b);
        CHECK(memcmp(hamlet, out, sizeof(hamlet)) == 0);
}

void test_lz77_no_overlap(void)
{
        static const uint8_t s[] = "aaaa"
                                   "aaaa"
                                   "aaaa";

        block_t b;

        /* With overlap. */
        b.cap = MAX_BLOCK_CAP;
        b.n = 0;
        CHECK(lz77_compress(s, 12, 32768, 258, true,
                            &output_lit, &output_backref, &b));
        CHECK(b.n == 2);
        CHECK(b.data[0].dist == 0 && b.data[0].litlen == 'a');
        CHECK(b.data[1].dist == 1 && b.data[1].litlen == 11);

        /* Without overlap. */
        b.cap = MAX_BLOCK_CAP;
        b.n = 0;
        CHECK(lz77_compress(s, 12, 32768, 258, false,
                            &output_lit, &output_backref, &b));
        CHECK(b.n == 7);
        CHECK(b.data[0].dist == 0 && b.data[0].litlen == 'a');
        CHECK(b.data[1].dist == 0 && b.data[1].litlen == 'a');
        CHECK(b.data[2].dist == 0 && b.data[2].litlen == 'a');
        CHECK(b.data[3].dist == 0 && b.data[3].litlen == 'a');
        CHECK(b.data[4].dist == 0 && b.data[4].litlen == 'a');
        CHECK(b.data[5].dist == 0 && b.data[5].litlen == 'a');
        CHECK(b.data[6].dist == 6 && b.data[6].litlen == 6);
}

void test_lz77_max_len(void)
{
        uint8_t src[100];
        block_t b;

        memset(src, 'x', sizeof(src));

        b.cap = MAX_BLOCK_CAP;
        b.n = 0;
        CHECK(lz77_compress(src, sizeof(src), 32768, 25, true,
                            &output_lit, &output_backref, &b));

        CHECK(b.n == 5);
        CHECK(b.data[0].dist == 0 && b.data[0].litlen == 'x');
        CHECK(b.data[1].dist == 1 && b.data[1].litlen == 25);
        CHECK(b.data[2].dist == 1 && b.data[2].litlen == 25);
        CHECK(b.data[3].dist == 1 && b.data[3].litlen == 25);
        CHECK(b.data[4].dist == 1 && b.data[4].litlen == 24);
}

void test_lz77_max_dist(void)
{
        static const uint8_t s[18] = "1234"
                                     "xxxxxxxxxx"
                                     "1234";
        block_t b;

        b.cap = MAX_BLOCK_CAP;


        /* Max dist: 14. */
        b.n = 0;
        CHECK(lz77_compress(s, sizeof(s), 14, 258, true,
                            &output_lit, &output_backref, &b));

        CHECK(b.n == 7);
        CHECK(b.data[0].dist == 0 && b.data[0].litlen == '1');
        CHECK(b.data[1].dist == 0 && b.data[1].litlen == '2');
        CHECK(b.data[2].dist == 0 && b.data[2].litlen == '3');
        CHECK(b.data[3].dist == 0 && b.data[3].litlen == '4');

        CHECK(b.data[4].dist == 0 && b.data[4].litlen == 'x');
        CHECK(b.data[5].dist == 1 && b.data[5].litlen == 9);

        CHECK(b.data[6].dist == 14 && b.data[6].litlen == 4);

        /* Max dist: 13. */
        b.n = 0;
        CHECK(lz77_compress(s, sizeof(s), 13, 258, true,
                            &output_lit, &output_backref, &b));

        CHECK(b.n == 10);
        CHECK(b.data[0].dist == 0 && b.data[0].litlen == '1');
        CHECK(b.data[1].dist == 0 && b.data[1].litlen == '2');
        CHECK(b.data[2].dist == 0 && b.data[2].litlen == '3');
        CHECK(b.data[3].dist == 0 && b.data[3].litlen == '4');

        CHECK(b.data[4].dist == 0 && b.data[4].litlen == 'x');
        CHECK(b.data[5].dist == 1 && b.data[5].litlen == 9);

        CHECK(b.data[6].dist == 0 && b.data[6].litlen == '1');
        CHECK(b.data[7].dist == 0 && b.data[7].litlen == '2');
        CHECK(b.data[8].dist == 0 && b.data[8].litlen == '3');
        CHECK(b.data[9].dist == 0 && b.data[9].litlen == '4');
}
