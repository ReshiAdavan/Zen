#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include "log.h"
#include "config.h"

/* Global file handle for logging on disk */
static FILE *fh = NULL;

/*
 * Tries to open in append mode a file on disk, to be called only if logging to
 * disk is active
 */
void zen_log_init(const char *file) {
    if (!file) 
        return;
    fh = fopen(file, "a+");
    if (!fh)
        printf("%lu WARNING: Unable to open file %s: %s\n", (unsigned long) time(NULL), file, strerror(errno));
}

/*
 * Close the previously opened file handler, to be called only after
 * zen_log_init has been called
 */
void zen_log_close(void) {
    if (fh) {
        fflush(fh);
        fclose(fh);
    }
}

void zen_log(int level, const char *fmt, ...) {

    if (level < conf->loglevel)
        return;

    assert(fmt);

    va_list ap;
    char msg[MAX_LOG_SIZE + 4];

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    /* Truncate message too long and copy 3 bytes to make space for 3 dots */
    memcpy(msg + MAX_LOG_SIZE, "...", 3);
    msg[MAX_LOG_SIZE + 3] = '\0';

    // Open two handler, one for standard output and a second for the
    // persistent log file
    FILE *fp = stdout;

    if (!fp)
        return;

    fprintf(fp, "%lu %s\n", (unsigned long) time(NULL), msg);
    if (fh)
        fprintf(fh, "%lu %s\n", (unsigned long) time(NULL), msg);
}
