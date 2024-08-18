/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "hamlet.h"
#include "implode.h"

void *bench_implode_hamlet_setup(void)
{
        return malloc(sizeof(hamlet));
}

void bench_implode_hamlet(void *aux)
{
        uint8_t *dst = aux;
        size_t dst_used;

        hwimplode(hamlet, sizeof(hamlet), true, true,
                  dst, sizeof(hamlet), &dst_used);
}

void bench_implode_hamlet_teardown(void *aux)
{
        free(aux);
}

size_t bench_implode_hamlet_bytes(void)
{
        return sizeof(hamlet);
}


struct explode_data {
        uint8_t *compressed;
        size_t compressed_size;
        uint8_t *decompressed;
};

void *bench_explode_hamlet_setup(void)
{
        struct explode_data *d;

        d = malloc(sizeof(*d));
        d->compressed = malloc(sizeof(hamlet));
        d->decompressed = malloc(sizeof(hamlet));

        hwimplode(hamlet, sizeof(hamlet), true, true,
                 d->compressed, sizeof(hamlet), &d->compressed_size);

        return d;
}

void bench_explode_hamlet(void *aux)
{
        struct explode_data *d = aux;
        explode_stat_t s;
        size_t src_used;

        s = hwexplode(d->compressed, d->compressed_size, sizeof(hamlet),
                      true, true, false, &src_used, d->decompressed);
        assert(s == HWEXPLODE_OK);
        assert(src_used == d->compressed_size);
        assert(memcmp(hamlet, d->decompressed, sizeof(hamlet)) == 0);
        (void)s;
}

void bench_explode_hamlet_teardown(void *aux)
{
        struct explode_data *d = aux;

        free(d->compressed);
        free(d->decompressed);
        free(d);
}

size_t bench_explode_hamlet_bytes(void)
{
        return sizeof(hamlet);
}
