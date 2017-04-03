include make_include

SUBDIRS = AttrProj Birch

birch: 
	@set -e ; for i in $(SUBDIRS) ; do ( cd $$i && make all ; ) ; done
	$(CPLUS) $(CPLUSFLAGS) Birch/BirchLib.o AttrProj/AttrProjLib.o $(LIBDIR) -o birch -lm

clean: FORCE
	@set -e ; for i in $(SUBDIRS) ; do ( cd $$i && make clean ; ) ; done
	rm -f birch

.PHONY: FORCE all clean
