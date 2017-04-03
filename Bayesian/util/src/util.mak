#-----------------------------------------------------------------------
# File    : util.mak
# Contents: build utility modules
# Author  : Christian Borgelt
# History : 26.01.2003 file created
#           05.06.2003 module params added
#           12.08.2003 module nstats added
#-----------------------------------------------------------------------
CC      = cl.exe
LD      = link.exe
DEFS    = /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS"
CFLAGS  = /nologo /ML /W3 /GX /O2 $(DEFS) /YX /FD /c

#-----------------------------------------------------------------------
# Build Modules
#-----------------------------------------------------------------------
all:        vecops.obj listops.obj symtab.obj nimap.obj tfscan.obj \
            scform.obj scan.obj

#-----------------------------------------------------------------------
# Vector Operations
#-----------------------------------------------------------------------
vecops.obj:   vecops.h vecops.c util.mak
	$(CC) $(CFLAGS) vecops.c /Fo$@

#-----------------------------------------------------------------------
# List Operations
#-----------------------------------------------------------------------
listops.obj:  listops.h listops.c util.mak
	$(CC) $(CFLAGS) listops.c /Fo$@

#-----------------------------------------------------------------------
# Numerical Statistics
#-----------------------------------------------------------------------
nstats.obj:   nstats.h nstats.c util.mak
	$(CC) $(CFLAGS) nstats.c /Fo$@

#-----------------------------------------------------------------------
# Symbol Table Management
#-----------------------------------------------------------------------
symtab.obj:   symtab.h symtab.c util.mak
	$(CC) $(CFLAGS) symtab.c /Fo$@

nimap.obj:    symtab.h vecops.h symtab.c util.mak
	$(CC) $(CFLAGS) /D "NIMAPFN" symtab.c /Fo$@

#-----------------------------------------------------------------------
# Table File Scanner Management
#-----------------------------------------------------------------------
tfscan.obj:   tfscan.h tfscan.c util.mak
	$(CC) $(CFLAGS) tfscan.c /Fo$@

#-----------------------------------------------------------------------
# Scanner
#-----------------------------------------------------------------------
scform.obj:   scan.h scan.c util.mak
	$(CC) $(CFLAGS) scan.c /Fo$@

scan.obj:     scan.h scan.c util.mak
	$(CC) $(CFLAGS) /D SC_SCAN scan.c /Fo$@

#-----------------------------------------------------------------------
# Command Line Parameter Retrieval
#-----------------------------------------------------------------------
params.o:   params.h
params.o:   params.c makefile
	$(CC) $(CFLAGS) -c params.c -o $@

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------
clean:
	-@erase /Q *~ *.obj *.idb *.pch
