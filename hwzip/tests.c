/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "test_utils.h"

#define TESTS \
TEST(bits_lsb) \
TEST(bits_read16le) \
TEST(bits_read32le) \
TEST(bits_read64le) \
TEST(bits_reverse16) \
TEST(bits_round_up) \
TEST(bits_write16le) \
TEST(bits_write32le) \
TEST(bits_write64le) \
TEST(crc32_basic) \
TEST(deflate_basic) \
TEST(deflate_hamlet) \
TEST(deflate_mixed_blocks) \
TEST(deflate_random) \
TEST(deflate_no_dist_codes) \
TEST(expand_hamlet2048) \
TEST(expand_overlap) \
TEST(expand_too_short) \
TEST(expand_too_short2) \
TEST(expand_zeros) \
TEST(explode_bad_huffman_trees) \
TEST(explode_hamlet_256) \
TEST(explode_hamlet_8192) \
TEST(explode_implicit_zeros) \
TEST(explode_pkz_101_bug_compat) \
TEST(explode_too_few_codeword_lens) \
TEST(explode_too_long_backref) \
TEST(explode_too_many_codeword_lens) \
TEST(explode_too_short_src) \
TEST(huffman_decode_basic) \
TEST(huffman_decode_canonical) \
TEST(huffman_decode_empty) \
TEST(huffman_decode_evil) \
TEST(huffman_decode_max_bits) \
TEST(huffman_encoder_init) \
TEST(huffman_encoder_init2) \
TEST(huffman_encoder_max_bits) \
TEST(huffman_lengths_limited) \
TEST(huffman_lengths_max_freq) \
TEST(huffman_lengths_max_syms) \
TEST(huffman_lengths_none) \
TEST(huffman_lengths_one) \
TEST(huffman_lengths_two) \
TEST(implode_hamlet_size) \
TEST(implode_roundtrip_basic) \
TEST(implode_roundtrip_empty) \
TEST(implode_roundtrip_hamlet) \
TEST(implode_roundtrip_long_backref) \
TEST(implode_too_short_dst) \
TEST(inflate_hamlet) \
TEST(inflate_invalid_block_header) \
TEST(inflate_no_dist_codes) \
TEST(inflate_twocities_intro) \
TEST(inflate_uncompressed) \
TEST(istream_basic) \
TEST(istream_min_bits) \
TEST(lz77_aaa) \
TEST(lz77_backref) \
TEST(lz77_chain) \
TEST(lz77_deferred) \
TEST(lz77_empty) \
TEST(lz77_hamlet) \
TEST(lz77_literals) \
TEST(lz77_max_dist) \
TEST(lz77_max_len) \
TEST(lz77_no_overlap) \
TEST(lz77_output_fail) \
TEST(lz77_outside_window) \
TEST(lz77_remaining_backref) \
TEST(ostream_basic) \
TEST(ostream_write_bytes_aligned) \
TEST(reduce_bad_follower_index) \
TEST(reduce_hamlet_size) \
TEST(reduce_implicit_zero_follower) \
TEST(reduce_max_follower_set_size) \
TEST(reduce_roundtrip_dle) \
TEST(reduce_roundtrip_empty) \
TEST(reduce_roundtrip_hamlet) \
TEST(reduce_too_short) \
TEST(shrink_aaa) \
TEST(shrink_basic) \
TEST(shrink_empty) \
TEST(shrink_hamlet) \
TEST(shrink_many_codes) \
TEST(unshrink_bad_control_code) \
TEST(unshrink_basic) \
TEST(unshrink_early_snag) \
TEST(unshrink_empty) \
TEST(unshrink_invalidated_prefix_codes) \
TEST(unshrink_lzw_fig5) \
TEST(unshrink_self_prefix) \
TEST(unshrink_snag) \
TEST(unshrink_too_short) \
TEST(unshrink_tricky_first_code) \
TEST(wildcard_match) \
TEST(zip_bad_stored_uncomp_size) \
TEST(zip_basic) \
TEST(zip_empty) \
TEST(zip_in_zip) \
TEST(zip_legacy_implode) \
TEST(zip_many_methods) \
TEST(zip_max_comment) \
TEST(zip_mtime) \
TEST(zip_out_of_bounds_member) \
TEST(zip_pk) \
TEST(zip_write_basic) \
TEST(zip_write_empty) \
TEST(zip_write_methods) \
TEST(zip_write_empty_member)

/* Declarations of the test functions. */
#define TEST(name) extern void test_ ## name(void);
TESTS
#undef TEST

/* Array of test names and functions. */
static struct {
        const char *name;
        void (*func)(void);
} const tests[] = {
#define TEST(name) { #name, test_ ## name },
TESTS
#undef TEST
};

int main(int argc, char **argv) {
        bool found;
        size_t i;

        if (argc != 1 && argc != 2) {
                printf("Usage: %s [testname]\n", argv[0]);
                return 1;
        }

        found = false;
        for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
                if (argc == 2 && !wildcard_match(argv[1], tests[i].name)) {
                        continue;
                }
                found = true;
                printf("%-36s", tests[i].name);
                fflush(stdout);
                (tests[i].func)();
                printf(" pass\n");
        }

        if (!found) {
                printf("Test '%s' not found!\n", argv[1]);
                return 1;
        }

        return 0;
}
