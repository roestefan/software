
#include <Python.h>

void python_osd_init(void);

void python_osd_reset(int halt);
void python_osd_start(void);
void python_osd_wait(int secs);

PyObject *python_osd_get_num_modules(void);
PyObject *python_osd_get_module_name(uint16_t id);

int python_osd_mem_loadelf(size_t modid, char* filename);
int python_osd_stm_log(size_t modid, char* filename);
int python_osd_ctm_log(size_t modid, char* filename);
int python_osd_ctm_log_symbols(size_t modid, char* filename, char* elffile);
