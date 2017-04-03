#-----------------------------------------------------------------------
# File    : makefile    (directory: matrix)
# Contents: build vector and matrix management modules
# Author  : Christian Borgelt
# History : 19.05.1999 file created
#           29.04.2001 module split into 3 parts
#           17.10.2001 programs transp, invert, solve added
#-----------------------------------------------------------------------
include ../../make.common

CFBASE    = -g -O2 -ansi -Wall -pedantic $(ADDFLAGS)
CFLAGS    = $(CFBASE) -DNDEBUG -O3
# CFLAGS    = $(CFBASE) -g
# CFLAGS    = $(CFBASE) -g -DSTORAGE
INC       = -I$(UTILDIR) $(ADDINC)
LIBS      = -lm
# LIBS      = -lm -lefence
# ADDINC    = -I../../misc/src
# ADDOBJ    = storage.o

UTILDIR   = ../../util/src
OBJ1_O    = mat_read.o $(UTILDIR)/tfscan.o
OBJ2_O    = mat_read.o matrix2.o $(UTILDIR)/tfscan.o
PRGS      = transp invert solve

#-----------------------------------------------------------------------
# Build Programs
#-----------------------------------------------------------------------
all:        $(PRGS)

transp:     $(OBJ1_O) transp.o
	$(CC) $(LDFLAGS) $(OBJ1_O) transp.o $(LIBS) -o $@

invert:     $(OBJ2_O) invert.o
	$(CC) $(LDFLAGS) $(OBJ2_O) invert.o $(LIBS) -o $@

solve:      $(OBJ2_O) solve.o
	$(CC) $(LDFLAGS) $(OBJ2_O) solve.o  $(LIBS) -o $@

#-----------------------------------------------------------------------
# Main Programs
#-----------------------------------------------------------------------
transp.o:   matrix.h
transp.o:   transp.c
	$(CC) $(CFLAGS) $(INC) -c transp.c -o $@

invert.o:   matrix.h
invert.o:   invert.c
	$(CC) $(CFLAGS) $(INC) -c invert.c -o $@

solve.o:    matrix.h
solve.o:    solve.c
	$(CC) $(CFLAGS) $(INC) -c solve.c -o $@

#-----------------------------------------------------------------------
# Vector and Matrix Management
#-----------------------------------------------------------------------
matrix1.o:  matrix.h
matrix1.o:  matrix1.c
	$(CC) $(CFLAGS) $(INC) -c matrix1.c -o $@

mat_read.o: matrix.h $(UTILDIR)/tfscan.h
mat_read.o: matrix1.c
	$(CC) $(CFLAGS) $(INC) -DMAT_READ -c matrix1.c -o $@

matrix2.o:  matrix.h
matrix2.o:  matrix2.c
	$(CC) $(CFLAGS) $(INC) -c matrix2.c -o $@

matrix3.o:  matrix.h
matrix3.o:  matrix3.c
	$(CC) $(CFLAGS) $(INC) -c matrix3.c -o $@

#-----------------------------------------------------------------------
# External Modules
#-----------------------------------------------------------------------
$(UTILDIR)/tfscan.o:
	cd $(UTILDIR); $(MAKE) tfscan.o ADDFLAGS=$(ADDFLAGS)

#-----------------------------------------------------------------------
# Storage Debugging
#-----------------------------------------------------------------------
storage.o:  ../../misc/src/storage.h
storage.o:  ../../misc/src/storage.c
	$(CC) $(CFLAGS) -c ../../misc/src/storage.c -o $@

#-----------------------------------------------------------------------
# Install
#-----------------------------------------------------------------------
install:
	cp $(PRGS) $(HOME)/bin

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------
clean:
	rm -f *.o *~ *.flc core $(PRGS)
	cd $(UTILDIR); $(MAKE) clean

localclean:
	rm -f *.o *~ *.flc core $(PRGS)
