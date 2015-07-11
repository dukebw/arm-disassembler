#ifndef STOPIF_H
#define STOPIF_H

#include <stdio.h>
#include <stdlib.h> // abort

// NOTE(brendan): source: 21st Century C by Ben Klemens

/** Set this to \c 's' to stop the program on an error.
    Otherwise, functcions return a value on failure. */
static char error_mode = 's';

/** To where should I write errors? If this is \c NULL, write to \c stderr. */
static FILE *error_log = 0;

// NOTE(brendan): can make a debug switch to turn this off in release builds
#define Stopif(assertion, error_action, ...)                      \
        if (assertion) {                                          \
            fprintf(error_log ? error_log : stderr, __VA_ARGS__); \
            fprintf(error_log ? error_log : stderr, "\n");        \
            if (error_mode == 's') {                              \
                abort();                                          \
            }                                                     \
            else {                                                \
                error_action;                                     \
            }                                                     \
        }

#endif /* STOPIF_H */
