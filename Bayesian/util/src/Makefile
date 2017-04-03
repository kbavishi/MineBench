#-----------------------------------------------------------------------
# File    : makefile    (directory: util)
# Contents: build utility modules
# Author  : Christian Borgelt
# History : 04.11.2000 file created from makefile in table directory
#           05.06.2003 module params added
#           12.08.2003 module nstats added
#-----------------------------------------------------------------------
include ../../make.common

CFBASE  = -g -O2 -ansi -Wall -pedantic $(ADDFLAGS)
CFLAGS  = $(CFBASE) -DNDEBUG -O3
# CFLAGS  = $(CFBASE) -g
# CFLAGS  = $(CFBASE) -g -DSTORAGE $(ADDINC)
# ADDINC  = -I../../misc/src

#-----------------------------------------------------------------------
# Build Modules
#-----------------------------------------------------------------------
all:        vecops.o listops.o symtab.o nimap.o tfscan.o scform.o \
            scan.o params.o

#-----------------------------------------------------------------------
# Vector Operations
#-----------------------------------------------------------------------
vecops.o:   vecops.h
vecops.o:   vecops.c
	$(CC) $(CFLAGS) -c vecops.c -o $@

#-----------------------------------------------------------------------
# List Operations
#-----------------------------------------------------------------------
listops.o:  listops.h
listops.o:  listops.c
	$(CC) $(CFLAGS) -c listops.c -o $@

#-----------------------------------------------------------------------
# Symbol Table Management
#-----------------------------------------------------------------------
symtab.o:   symtab.h
symtab.o:   symtab.c
	$(CC) $(CFLAGS) -c symtab.c -o $@

nimap.o:    symtab.h vecops.h
nimap.o:    symtab.c
	$(CC) $(CFLAGS) -DNIMAPFN -c symtab.c -o $@

#-----------------------------------------------------------------------
# Numerical Statistics Management
#-----------------------------------------------------------------------
nstats.o:   nstats.h
nstats.o:   nstats.c
	$(CC) $(CFLAGS) -c nstats.c -o $@

#-----------------------------------------------------------------------
# Table File Scanner Management
#-----------------------------------------------------------------------
tfscan.o:   tfscan.h
tfscan.o:   tfscan.c
	$(CC) $(CFLAGS) -c tfscan.c -o $@

#-----------------------------------------------------------------------
# Scanner
#-----------------------------------------------------------------------
scform.o:   scan.h
scform.o:   scan.c
	$(CC) $(CFLAGS) -c scan.c -o $@

scan.o:     scan.h
scan.o:     scan.c
	$(CC) $(CFLAGS) -DSC_SCAN -c scan.c -o $@

#-----------------------------------------------------------------------
# Command Line Parameter Retrieval
#-----------------------------------------------------------------------
params.o:   params.h
params.o:   params.c
	$(CC) $(CFLAGS) -c params.c -o $@

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------
clean:
	rm -f *.o *~ *.flc core
