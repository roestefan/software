
SWIGFILE = opensocdebug.i
SWIGGEN=opensocdebug_wrap.c opensocdebug.py


all-local: $(SWIGGEN)
	$(PYTHON) setup.py build

install-exec-local:
	$(PYTHON) setup.py install	

$(SWIGGEN): $(SWIGFILE)
	swig -python -o opensocdebug_wrap.c -oh opensocdebug_wrap.h -outdir . $(srcdir)/$(SWIGFILE)
	
EXTRA_DIST = $(SWIGFILE)