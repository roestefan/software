/* File: example.c */

#include "osd_python.h"

#include <opensocdebug.h>

struct osd_context *ctx = 0;

void init() {
    if (ctx == 0) {
        osd_new(&ctx, OSD_MODE_DAEMON, 0, 0);

        if (osd_connect(ctx) != OSD_SUCCESS) {
            printf("Cannot connect\n");
            exit(1);
        }

    }
}

void python_osd_reset(int halt) {
    printf("reset\n");
    osd_reset_system(ctx, halt);
}
