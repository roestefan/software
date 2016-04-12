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
void start(void);
void wait(int secs);

PyObject *python_osd_get_memories();

int python_osd_mem_loadelf(size_t modid, char* filename);

%pythoncode %{
memories = []

class Memory(object):
	modid = None

	def __init__(self, mod):
		self.modid = mod

	def loadelf(self, filename):
		return python_osd_mem_loadelf(self.modid, filename)

def reset(halt=False):
	python_osd_reset(halt);
	
def get_memories():
    return memories

for m in python_osd_get_memories():
    mem = Memory(m)
    memories.append(mem)

%}
