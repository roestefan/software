/* File: example.i */
%module opensocdebug

%{
#define SWIG_FILE_WITH_INIT
#include "osd_python.h"
%}

%init %{
    init();
%}

void python_osd_reset(int halt);

%pythoncode %{

def reset(halt=False):
	python_osd_reset(halt);
	
%}