/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "hamlet.h"
#include "reduce.h"

void *bench_reduce_hamlet_setup(void)
{
        return malloc(sizeof(hamlet));
}

void bench_reduce_hamlet(void *aux)
{
        uint8_t *dst = aux;
        size_t dst_used;

        hwreduce(hamlet, sizeof(hamlet), 4, dst, sizeof(hamlet), &dst_used);
}

void bench_reduce_hamlet_teardown(void *aux)
{
        free(aux);
}

size_t bench_reduce_hamlet_bytes(void)
{
        return sizeof(hamlet);
}


struct expand_data {
        uint8_t *compressed;
        size_t compressed_size;
        uint8_t *decompressed;
};

void *bench_expand_hamlet_setup(void)
{
        struct expand_data *d;

        d = malloc(sizeof(*d));
        d->compressed = malloc(sizeof(hamlet));
        d->decompressed = malloc(sizeof(hamlet));

        hwreduce(hamlet, sizeof(hamlet), 4,
                 d->compressed, sizeof(hamlet), &d->compressed_size);

        return d;
}

void bench_expand_hamlet(void *aux)
{
        struct expand_data *d = aux;
        expand_stat_t s;
        size_t src_used;

        s = hwexpand(d->compressed, d->compressed_size, sizeof(hamlet), 4,
                     &src_used, d->decompressed);
        assert(s == HWEXPAND_OK);
        assert(src_used == d->compressed_size);
        assert(memcmp(hamlet, d->decompressed, sizeof(hamlet)) == 0);
        (void)s;
}

void bench_expand_hamlet_teardown(void *aux)
{
        struct expand_data *d = aux;

        free(d->compressed);
        free(d->decompressed);
        free(d);
}

size_t bench_expand_hamlet_bytes(void)
{
        return sizeof(hamlet);
}
