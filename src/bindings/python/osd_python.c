/* File: example.c */

#include "osd_python.h"

#include <opensocdebug.h>
#include <unistd.h>

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
    osd_reset_system(ctx, halt);
}

void start(void) {
    osd_start_cores(ctx);
}

void wait(int secs) {
    sleep(secs);
}

PyObject *python_osd_get_memories() {
    uint16_t *ids;
    size_t num;
    osd_get_memories(ctx, &ids, &num);

    PyObject* list = PyList_New(num);

    for (size_t i = 0; i < num; i++) {
        PyObject *obj = PyInt_FromLong(ids[i]);
        PyList_SetItem(list, i, obj);
    }

    return list;
}

int python_osd_mem_loadelf(size_t modid, char* filename) {
    return osd_memory_loadelf(ctx, modid, filename);
}
