/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "deflate.h"
#include "hamlet.h"
#include "test_utils.h"
#include "zlib_utils.h"

void *bench_deflate_hamlet_setup(void)
{
        return malloc(sizeof(hamlet));
}

void bench_deflate_hamlet(void *aux)
{
        uint8_t *dst = aux;
        size_t dst_used;

        hwdeflate(hamlet, sizeof(hamlet), dst, sizeof(hamlet), &dst_used);
}

void bench_deflate_hamlet_teardown(void *aux)
{
        free(aux);
}

size_t bench_deflate_hamlet_bytes(void)
{
        return sizeof(hamlet);
}


struct inflate_data {
        uint8_t *compressed;
        size_t compressed_size;
        uint8_t *decompressed;
};

void *bench_inflate_hamlet_setup(void)
{
        struct inflate_data *d;

        d = malloc(sizeof(*d));
        d->compressed = malloc(sizeof(hamlet));
        d->decompressed = malloc(sizeof(hamlet));

        d->compressed_size = zlib_deflate(hamlet, sizeof(hamlet), 9,
                                          d->compressed, sizeof(hamlet));

        return d;
}

void bench_inflate_hamlet(void *aux)
{
        struct inflate_data *d = aux;
        inf_stat_t s;
        size_t src_used, dst_len;

        s = hwinflate(d->compressed, d->compressed_size, &src_used,
                      d->decompressed, sizeof(hamlet), &dst_len);

        assert(s == HWINF_OK);
        assert(dst_len == sizeof(hamlet));
        assert(memcmp(d->decompressed, hamlet, sizeof(hamlet)) == 0);
        (void)s;
}

void bench_inflate_hamlet_teardown(void *aux)
{
        struct inflate_data *d = aux;

        free(d->compressed);
        free(d->decompressed);
        free(d);
}

size_t bench_inflate_hamlet_bytes(void)
{
        return sizeof(hamlet);
}


#define RANDOM_SIZE 250000

void *bench_deflate_random_setup(void)
{
        uint8_t *random;
        uint32_t x;
        size_t i;

        random = malloc(RANDOM_SIZE);

        x = 0;
        for (i = 0; i < RANDOM_SIZE; i++) {
                x = next_test_rand(x);
                random[i] = (uint8_t)(x >> 24);
        }

        return random;
}

void bench_deflate_random(void *aux)
{
        uint8_t dst[RANDOM_SIZE * 2];
        size_t dst_used;
        uint8_t *random = aux;
        bool ok;

        ok = hwdeflate(random, RANDOM_SIZE, dst, sizeof(dst), &dst_used);
        assert(ok);
        (void)ok;
}

void bench_deflate_random_teardown(void *aux)
{
        free(aux);
}

size_t bench_deflate_random_bytes(void)
{
        return RANDOM_SIZE;
}


void *bench_inflate_random_setup(void)
{
        struct inflate_data *d;
        uint8_t *random;

        random = bench_deflate_random_setup();

        d = malloc(sizeof(*d));
        d->compressed = malloc(2 * RANDOM_SIZE);
        d->decompressed = malloc(RANDOM_SIZE);

        d->compressed_size = zlib_deflate(random, RANDOM_SIZE, 9,
                                          d->compressed, 2 * RANDOM_SIZE);
        free(random);

        return d;
}

void bench_inflate_random(void *aux)
{
        struct inflate_data *d = aux;
        inf_stat_t s;
        size_t src_used, dst_len;

        s = hwinflate(d->compressed, d->compressed_size, &src_used,
                      d->decompressed, RANDOM_SIZE, &dst_len);

        assert(s == HWINF_OK);
        assert(dst_len == RANDOM_SIZE);
        (void)s;
}

void bench_inflate_random_teardown(void *aux)
{
        struct inflate_data *d = aux;

        free(d->compressed);
        free(d->decompressed);
        free(d);
}

size_t bench_inflate_random_bytes(void)
{
        return RANDOM_SIZE;
}
