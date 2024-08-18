# This file is part of hwzip from https://www.hanshq.net/zip.html
# It is put in the public domain; see the LICENSE file for details.

# Note: The official build instructions are in the README file.
# This Makefile is used during development, and you will most likely want
# to tweak some flags below when using it. In particular, you may not
# want to use -Weverything.
#
# Don't forget to pass OPT=1 for optimized builds.


CC = clang
LD = $(CC)

WFLAGS = -Weverything -Wno-missing-prototypes -Wno-missing-noreturn \
         -Wno-overlength-strings -Wno-language-extension-token -Wno-padded \
         -Wno-c99-extensions -Wno-unsafe-buffer-usage -Wno-long-long \
         -Wno-switch-default

CFLAGS  = -std=gnu90 -MMD $(WFLAGS)
LDFLAGS =

FUZZFLAGS = $(CFLAGS) -fsanitize=address,undefined,fuzzer \
            -fno-sanitize-recover=all -g -O3 -march=native -UNDEBUG

ifeq ($(WERROR),1)
WFLAGS += -Werror
endif

ifeq ($(OPT),1)
CFLAGS += -O3 -DNDEBUG -march=native -Wno-unused-macros
else
CFLAGS += -O0 -g -gdwarf-4
endif

ifeq ($(M32),1)
CFLAGS  += -m32
LDFLAGS += -m32
endif

ifeq ($(ASAN),1)
CFLAGS  += -fsanitize=address,undefined -fno-sanitize-recover=all
LDFLAGS += -fsanitize=address,undefined -fno-sanitize-recover=all
endif

ifeq ($(VALGRIND),1)
Vgrnd=valgrind --quiet --error-exitcode=1 --leak-check=full
else
Vgrnd=
endif

ifeq ($(COVERAGE),1)
CFLAGS  += -fprofile-instr-generate -fcoverage-mapping
LDFLAGS += -fprofile-instr-generate
GenCov = llvm-profdata merge default.profraw -o default.profdata && \
         llvm-cov show -format html -instr-profile default.profdata \
         -o cov -show-line-counts-or-regions
GenCovCheck = $(GenCov) tests
GenCovBench = $(GenCov) benchmarks
else
GenCovCheck =
GenCovBench =
endif

FUZZOPTS = -max_len=1000000 -rss_limit_mb=5000
ifeq ($(FUZZLIMIT),1)
FUZZOPTS += -max_total_time=10
endif


# Source files shared between targets.
CORESRC  = crc32.c
CORESRC += deflate.c
CORESRC += huffman.c
CORESRC += implode.c
CORESRC += lz77.c
CORESRC += reduce.c
CORESRC += shrink.c
CORESRC += tables.c
CORESRC += zip.c

# Source files for the 'hwzip' target.
HWZIPSRC  = $(CORESRC)
HWZIPSRC += hwzip.c

# Source files for the 'tests' target.
TESTSRC  = $(CORESRC)
TESTSRC += bits_test.c
TESTSRC += bitstream_test.c
TESTSRC += crc32_test.c
TESTSRC += deflate_test.c
TESTSRC += hamlet.c
TESTSRC += huffman_test.c
TESTSRC += implode_test.c
TESTSRC += lz77_test.c
TESTSRC += reduce_test.c
TESTSRC += shrink_test.c
TESTSRC += test_utils.c
TESTSRC += test_utils_test.c
TESTSRC += tests.c
TESTSRC += zip_test.c
TESTSRC += zlib_utils.c

# Source files for the 'benchmarks' target.
BENCHSRC  = $(CORESRC)
BENCHSRC += benchmarks.c
BENCHSRC += deflate_bench.c
BENCHSRC += hamlet.c
BENCHSRC += huffman_bench.c
BENCHSRC += implode_bench.c
BENCHSRC += lz77_bench.c
BENCHSRC += reduce_bench.c
BENCHSRC += shrink_bench.c
BENCHSRC += test_utils.c
BENCHSRC += zlib_bench.c
BENCHSRC += zlib_utils.c

# Fuzzers.
FUZZERS  = deflate_fuzzer.c
FUZZERS += expand_fuzzer.c
FUZZERS += explode_fuzzer.c
FUZZERS += huffman_decode_fuzzer.c
FUZZERS += huffman_encode_fuzzer.c
FUZZERS += implode_fuzzer.c
FUZZERS += inflate_fuzzer.c
FUZZERS += reduce_fuzzer.c
FUZZERS += shrink_fuzzer.c
FUZZERS += unshrink_fuzzer.c
FUZZERS += zip_fuzzer.c

ALLSRC = $(HWZIPSRC) $(TESTSRC) $(BENCHSRC) $(FUZZERS) generate_tables.c

all : hwzip tests benchmarks fuzzers

check : tests
	$(Vgrnd) ./tests
	$(GenCovCheck)

bench : benchmarks
	$(Vgrnd) ./benchmarks
	$(GenCovBench)

%.o : %.c .flags
	$(CC) -c $< -o $@ $(CFLAGS)

tables.c : generate_tables.c .flags
	$(CC) -c $< -o generate_tables.o $(CFLAGS)
	$(LD) -o generate_tables generate_tables.o $(LDFLAGS)
	./generate_tables > $@

hamlet.c : hamlet.txt
	echo '#include "hamlet.h"' > $@
	echo 'const uint8_t hamlet[] = {' >> $@
	xxd -i < $< >> $@
	echo '};' >> $@

hwzip : $(HWZIPSRC:.c=.o)
	$(LD) -o $@ $^ $(LDFLAGS)

tests : $(TESTSRC:.c=.o)
	$(LD) -o $@ $^ $(LDFLAGS) -lz

benchmarks : $(BENCHSRC:.c=.o)
	$(LD) -o $@ $^ $(LDFLAGS) -lz

%.fuzz.o : %.c .flags
	$(CC) -c $< -o $@ $(FUZZFLAGS)

%_fuzzer : %_fuzzer.fuzz.o $(CORESRC:.c=.fuzz.o) zlib_utils.fuzz.o
	$(LD) -o $@ $^ $(LDFLAGS) -fsanitize=address,undefined,fuzzer -lz

%_fuzz : %_fuzzer
	mkdir -p $*_corpus/
	ASAN_OPTIONS=allocator_may_return_null=1 ./$< $(FUZZOPTS) $*_corpus/

fuzzers : $(FUZZERS:.c=)

fuzz_all : $(FUZZERS:_fuzzer.c=_fuzz)

clean :
	rm -f $(ALLSRC:.c=.o) $(ALLSRC:.c=.d)
	rm -f $(ALLSRC:.c=.fuzz.o) $(ALLSRC:.c=.fuzz.d)
	rm -f hwzip tests benchmarks $(FUZZERS:.c=)
	rm -f generate_tables hamlet.c tables.c
	rm -f default.profdata default.profdata default.profraw
	rm -rf cov
	rm -f .flags

.flags : dummy
	@{ \
          T=`mktemp`; \
          echo $(CC) $(CFLAGS) $(LD) $(LDFLAGS) $(FUZZFLAGS) > $$T; \
          cmp -s $$T $@ || mv -f $$T $@; \
        }

-include $(ALLSRC:.c=.d)

.PHONY : all check clean dummy

# Don't delete intermediary objects.
.SECONDARY :
