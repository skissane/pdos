/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#ifndef CC_DIAG_H
#define CC_DIAG_H 1

#ifndef CC_READER_DEFINED
#   define CC_READER_DEFINED 1
typedef struct cc_reader cc_reader;
#endif

#define TTY_RED     "\x1B[0;91m"
#define TTY_RESET   "\x1B[0;0m"
#define TTY_BOLD    "\x1B[1m"

typedef struct {
    const char *file;
    unsigned long line;
    int column;
} location_t;

typedef enum cc_diagnostic_level {
    CC_DL_NOTE,
    CC_DL_WARNING,
    CC_DL_ERROR
} cc_diagnostic_level;

void cc_report(cc_reader *reader, enum cc_diagnostic_level level,
               const char *msg, ...);

#endif
