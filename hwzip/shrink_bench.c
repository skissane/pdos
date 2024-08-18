/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "hamlet.h"
#include "shrink.h"

void *bench_shrink_hamlet_setup(void)
{
        return malloc(sizeof(hamlet));
}

void bench_shrink_hamlet(void *aux)
{
        uint8_t *dst = aux;
        size_t dst_used;

        hwshrink(hamlet, sizeof(hamlet), dst, sizeof(hamlet), &dst_used);
}

void bench_shrink_hamlet_teardown(void *aux)
{
        free(aux);
}

size_t bench_shrink_hamlet_bytes(void)
{
        return sizeof(hamlet);
}


struct unshrink_data {
        uint8_t *compressed;
        size_t compressed_size;
        uint8_t *decompressed;
};

void *bench_unshrink_hamlet_setup(void)
{
        struct unshrink_data *d;

        d = malloc(sizeof(*d));
        d->compressed = malloc(sizeof(hamlet));
        d->decompressed = malloc(sizeof(hamlet));

        hwshrink(hamlet, sizeof(hamlet), d->compressed, sizeof(hamlet),
                 &d->compressed_size);

        return d;
}

void bench_unshrink_hamlet(void *aux)
{
        struct unshrink_data *d = aux;
        unshrnk_stat_t s;
        size_t src_used, dst_used;

        s = hwunshrink(d->compressed, d->compressed_size, &src_used,
                       d->decompressed, sizeof(hamlet), &dst_used);
        assert(s == HWUNSHRINK_OK);
        assert(src_used == d->compressed_size);
        assert(dst_used == sizeof(hamlet));
        assert(memcmp(hamlet, d->decompressed, sizeof(hamlet)) == 0);
        (void)s;
}

void bench_unshrink_hamlet_teardown(void *aux)
{
        struct unshrink_data *d = aux;

        free(d->compressed);
        free(d->decompressed);
        free(d);
}

size_t bench_unshrink_hamlet_bytes(void)
{
        return sizeof(hamlet);
}
