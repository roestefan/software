
#include <Python.h>

void init(void);
void python_osd_reset(int halt);
void start(void);
void wait(int secs);

PyObject *python_osd_get_memories(void);
int python_osd_mem_loadelf(size_t modid, char* filename);
