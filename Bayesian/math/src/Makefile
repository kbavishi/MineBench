#-----------------------------------------------------------------------
# File    : makefile    (directory: math)
# Contents: build math utility modules
# Author  : Christian Borgelt
# History : 04.07.2002 file created from makefile in util directory
#           19.05.2003 intexp, choose, zeta, quantile, and normd added
#           15.08.2003 module radfn added
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
all:        intexp.o choose.o gamma.o zeta.o quantile.o normd.o radfn.o

#-----------------------------------------------------------------------
# Mathematical Functions
#-----------------------------------------------------------------------
intexp.o:   intexp.h
intexp.o:   intexp.c
	$(CC) $(CFLAGS) -c intexp.c -o $@

choose.o:   choose.h
choose.o:   choose.c
	$(CC) $(CFLAGS) -c choose.c -o $@

gamma.o:    gamma.h
gamma.o:    gamma.c
	$(CC) $(CFLAGS) -c gamma.c -o $@

zeta.o:     zeta.h
zeta.o:     zeta.c
	$(CC) $(CFLAGS) -c zeta.c -o $@

quantile.o: quantile.h
quantile.o: quantile.c
	$(CC) $(CFLAGS) -c quantile.c -o $@

normd.o:    normd.h
normd.o:    normd.c
	$(CC) $(CFLAGS) -c normd.c -o $@

radfn.o:    radfn.h
radfn.o:    radfn.c
	$(CC) $(CFLAGS) -c radfn.c -o $@

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------
clean:
	rm -f *.o *~ *.flc core
