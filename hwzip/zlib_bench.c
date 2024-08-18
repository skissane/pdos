/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "hamlet.h"
#include "test_utils.h"
#include "zlib_utils.h"

#define DST_SZ (1024 * 1024)

void *bench_zlib_deflate_hamlet_setup(void)
{
        return malloc(DST_SZ);
}

void bench_zlib_deflate_hamlet(void *aux)
{
        uint8_t *dst = aux;

        zlib_deflate(hamlet, sizeof(hamlet), 9, dst, DST_SZ);
}

void bench_zlib_deflate_hamlet_teardown(void *aux)
{
        free(aux);
}

size_t bench_zlib_deflate_hamlet_bytes(void)
{
        return sizeof(hamlet);
}


void *bench_zlib_inflate_hamlet_setup(void)
{
        uint8_t *dst = malloc(DST_SZ * 2);

        zlib_deflate(hamlet, sizeof(hamlet), 9, dst, DST_SZ);

        return dst;
}

void bench_zlib_inflate_hamlet(void *aux)
{
        uint8_t *comp = aux;
        uint8_t *decomp = &comp[DST_SZ];
        size_t decomp_sz;

        decomp_sz = zlib_inflate(comp, DST_SZ, decomp, DST_SZ);

        assert(decomp_sz == sizeof(hamlet));
        assert(memcmp(decomp, hamlet, sizeof(hamlet)) == 0);
        (void)decomp_sz;
}

void bench_zlib_inflate_hamlet_teardown(void *aux)
{
        free(aux);
}

size_t bench_zlib_inflate_hamlet_bytes(void)
{
        return sizeof(hamlet);
}

#define RANDOM_SIZE 250000
extern void *bench_deflate_random_setup(void);

void *bench_zlib_deflate_random_setup(void)
{
        return bench_deflate_random_setup();
}

void bench_zlib_deflate_random(void *aux)
{
        uint8_t dst[RANDOM_SIZE * 2];
        size_t dst_used;
        uint8_t *random = aux;

        dst_used = zlib_deflate(random, RANDOM_SIZE, 9, dst, sizeof(dst));
        assert(dst_used > 0);
        (void)dst_used;
}

void bench_zlib_deflate_random_teardown(void *aux)
{
        free(aux);
}

size_t bench_zlib_deflate_random_bytes(void)
{
        return RANDOM_SIZE;
}


void *bench_zlib_inflate_random_setup(void)
{
        uint8_t *compressed;
        uint8_t *random;

        random = bench_zlib_deflate_random_setup();

        compressed = malloc(RANDOM_SIZE * 2);
        zlib_deflate(random, RANDOM_SIZE, 9, compressed, RANDOM_SIZE * 2);
        free(random);

        return compressed;
}

void bench_zlib_inflate_random(void *aux)
{
        static uint8_t out[RANDOM_SIZE];
        uint8_t *compressed = aux;
        size_t dst_len;

        dst_len = zlib_inflate(compressed, 2 * RANDOM_SIZE, out, sizeof(out));
        assert(dst_len == RANDOM_SIZE);
        (void)dst_len;
}

void bench_zlib_inflate_random_teardown(void *aux)
{
        free(aux);
}

size_t bench_zlib_inflate_random_bytes(void)
{
        return RANDOM_SIZE;
}
