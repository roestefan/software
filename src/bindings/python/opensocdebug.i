/* File: example.i */
%module(package="opensocdebug") interface

%{
#define SWIG_FILE_WITH_INIT
#include "osd_python.h"
%}

%init %{
    python_osd_init();
%}

void python_osd_reset(int halt);
void python_osd_start(void);
void python_osd_wait(int secs);

PyObject *python_osd_get_num_modules();
PyObject *python_osd_get_module_name(int id);

int python_osd_mem_loadelf(size_t modid, char* filename);
