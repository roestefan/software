
#include <Python.h>

void python_osd_init(void);

void python_osd_reset(int halt);
void python_osd_start(void);
void python_osd_wait(int secs);

PyObject *python_osd_get_memories(void);
int python_osd_mem_loadelf(size_t modid, char* filename);
