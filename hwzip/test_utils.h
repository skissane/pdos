/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#define CHECK(cond) \
        do { \
                if (cond) {} else { check_failed(__FILE__, __LINE__, #cond); } \
        } while (0)

void check_failed(const char *file, unsigned line, const char *cond);

uint32_t next_test_rand(uint32_t x);

bool wildcard_match(const char *wildcard_str, const char *normal_str);

#endif
