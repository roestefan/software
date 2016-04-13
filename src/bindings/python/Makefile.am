
SWIGFILE = opensocdebug.i
SWIGGEN = opensocdebug_wrap.c opensocdebug.py

PKGFILES = __init__.py session.py
PKGSRCS = $(addprefix opensocdebug/,$(PKGFILES))

all-local: $(SWIGGEN) $(PKGSRCS) setup.py
	$(PYTHON) setup.py build

install-exec-local: setup.py
	$(PYTHON) setup.py install	

$(SWIGGEN): $(SWIGFILE)
	swig -python -o opensocdebug_wrap.c -outdir opensocdebug $(srcdir)/$(SWIGFILE)

opensocdebug/%.py: $(srcdir)/opensocdebug/%.py
	cp $< $@
	
EXTRA_DIST = $(SWIGFILE) $(PKGSRCS)
