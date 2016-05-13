/* File: example.c */

#include "osd_python.h"

#include <opensocdebug.h>
#include <unistd.h>

struct osd_context *ctx = 0;

void python_osd_init() {
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

void python_osd_start(void) {
    osd_start_cores(ctx);
}

void python_osd_wait(int secs) {
    sleep(secs);
}

PyObject *python_osd_get_num_modules() {
    uint16_t n;
    osd_get_num_modules(ctx, &n);
    return PyInt_FromLong(n);
}

PyObject *python_osd_get_module_name(uint16_t id) {
    char *name;
    osd_get_module_name(ctx, id, &name);
    PyObject *str = PyString_FromString(name);
    free(name);
    return str;
}

int python_osd_mem_loadelf(size_t modid, char* filename) {
    return osd_memory_loadelf(ctx, modid, filename);
}

int python_osd_stm_log(size_t modid, char* filename) {
    return osd_stm_log(ctx, modid, filename);
}
