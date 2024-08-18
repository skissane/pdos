/* This file is part of hwzip from https://www.hanshq.net/zip.html
   It is put in the public domain; see the LICENSE file for details. */

#include "test_utils.h"

void test_wildcard_match(void)
{
        CHECK(wildcard_match("", ""));
        CHECK(wildcard_match("*", ""));
        CHECK(wildcard_match("**", ""));
        CHECK(!wildcard_match("?", ""));

        CHECK(wildcard_match("foo", "foo"));
        CHECK(wildcard_match("f?o", "fxo"));
        CHECK(wildcard_match("f*o", "fxo"));
        CHECK(wildcard_match("f*o", "fo"));
        CHECK(wildcard_match("f*o", "f123o"));

        CHECK(!wildcard_match("foo", "fo0"));
        CHECK(!wildcard_match("f?o", "fo"));
        CHECK(!wildcard_match("f*o", "f"));

        CHECK(wildcard_match("*", "abc"));
        CHECK(wildcard_match("???", "abc"));

        CHECK(!wildcard_match("*a", ""));
        CHECK(!wildcard_match("???", "ab"));
        CHECK(!wildcard_match("???", "abcd"));

        CHECK( wildcard_match("f*o*o*o*o*o*o*o*x", "foooooooooooooooooooooox"));
        CHECK(!wildcard_match("f*o*o*o*o*o*o*o*x", "fooooooooooooooooooooooy"));
}
