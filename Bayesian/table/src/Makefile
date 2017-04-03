#-----------------------------------------------------------------------
# File    : makefile    (directory: table)
# Contents: build table management modules and utility programs
# Author  : Christian Borgelt
# History : ??.??.1995 file created
#           26.08.1997 makefile splitted into subdirectory makefiles
#           04.01.1998 table file scanner management added
#           08.02.1998 scanner transferred from parse makefile
#           24.02.1998 skeleton programs transferred from skel makefile
#           25.02.1998 programs tmerge and tsplit added
#           05.02.1999 program tjoin added
#           13.02.1999 program tbal added
#           17.04.1999 input/output utility functions added
#           04.11.2000 modules vecops, symtab, and tfscan made external
#           24.06.2001 modules attest and table split into two files
#           08.11.2001 module tab4vis added
#           22.02.2002 module attset3 added, as_rwp removed
#           22.07.2003 program tnorm added
#           11.08.2003 program t1inn added
#-----------------------------------------------------------------------
include ../../make.common

CFBASE    = -g -O2 -ansi -Wall -pedantic $(ADDFLAGS)
CFLAGS    = $(CFBASE) -DNDEBUG -O3
# CFLAGS    = $(CFBASE) -g
# CFLAGS    = $(CFBASE) -g $(ADDINC) -DSTORAGE
LDFLAGS   = 
INC       = -I$(UTILDIR)
LIBS      = -lm
#ADDINC    = -I../../misc/src
#ADDOBJ    = storage.o

UTILDIR   = ../../util/src
HDRS      = $(UTILDIR)/vecops.h $(UTILDIR)/tfscan.h \
            $(UTILDIR)/scan.h   attset.h
OBJS      = $(UTILDIR)/vecops.o $(UTILDIR)/tfscan.o \
            $(UTILDIR)/scform.o attset1.o attset2.o $(ADDOBJ)
OBJS2     = $(UTILDIR)/vecops.o $(UTILDIR)/tfscan.o \
            $(UTILDIR)/scan.o attset1.o attset2.o $(ADDOBJ)
DOM_O     = $(OBJS) io.o dom.o
TMERGE_O  = $(OBJS) io.o tmerge.o
TSPLIT_O  = $(OBJS) table1.o io_tab.o tsplit.o
TJOIN_O   = $(OBJS) table1.o io_tab.o tjoin.o
TBAL_O    = $(OBJS) table1.o io_tab.o tbal.o
TNORM_O   = $(OBJS) table1.o io_tab.o tnorm.o
T1INN_O   = $(OBJS2) attset3.o attmap.o io.o t1inn.o
OPC_O     = $(OBJS) table1.o table2.o io_tab.o opc.o
XMAT_O    = $(UTILDIR)/symtab.o $(UTILDIR)/tfscan.o xmat.o $(ADDOBJ)
UVINS_O   = $(OBJS) table1.o io_tab.o uvins.o
SKEL1_O   = $(OBJS) table1.o io_tab.o skel1.o
SKEL2_O   = $(OBJS2) attset3.o table1.o io_tab.o skel2.o $(ADDOBJ)
PRGS      = dom opc tmerge tsplit tjoin tbal tnorm t1inn xmat uvins

#-----------------------------------------------------------------------
# Build Programs
#-----------------------------------------------------------------------
all:        $(PRGS)

dom:        $(DOM_O)
	$(CC) $(LDFLAGS) $(DOM_O) $(LIBS) -o $@

opc:        $(OPC_O)
	$(CC) $(LDFLAGS) $(OPC_O) $(LIBS) -o $@

tmerge:     $(TMERGE_O)
	$(CC) $(LDFLAGS) $(TMERGE_O) $(LIBS) -o $@

tsplit:     $(TSPLIT_O)
	$(CC) $(LDFLAGS) $(TSPLIT_O) $(LIBS) -o $@

tjoin:      $(TJOIN_O)
	$(CC) $(LDFLAGS) $(TJOIN_O) $(LIBS) -o $@

tbal:       $(TBAL_O)
	$(CC) $(LDFLAGS) $(TBAL_O) $(LIBS) -o $@

tnorm:      $(TNORM_O)
	$(CC) $(LDFLAGS) $(TNORM_O) $(LIBS) -o $@

t1inn:      $(T1INN_O)
	$(CC) $(LDFLAGS) $(T1INN_O) $(LIBS) -o $@

xmat:       $(XMAT_O)
	$(CC) $(LDFLAGS) $(XMAT_O) $(LIBS) -o $@

uvins:      $(UVINS_O)
	$(CC) $(LDFLAGS) $(UVINS_O) $(LIBS) -o $@

skel1:      $(SKEL1_O)
	$(CC) $(LDFLAGS) $(SKEL1_O) $(LIBS) -o $@

skel2:      $(SKEL2_O)
	$(CC) $(LDFLAGS) $(SKEL2_O) $(LIBS) -o $@

#-----------------------------------------------------------------------
# Main Programs
#-----------------------------------------------------------------------
dom.o:      $(HDRS) io.h
dom.o:      dom.c
	$(CC) $(CFLAGS) $(INC) -c dom.c -o $@

opc.o:      $(HDRS) table.h io.h
opc.o:      opc.c
	$(CC) $(CFLAGS) $(INC) -c opc.c -o $@

tmerge.o:   $(HDRS) io.h
tmerge.o:   tmerge.c
	$(CC) $(CFLAGS) $(INC) -c tmerge.c -o $@

tsplit.o:   $(HDRS) table.h io.h
tsplit.o:   tsplit.c
	$(CC) $(CFLAGS) $(INC) -c tsplit.c -o $@

tjoin.o:    $(HDRS) table.h io.h
tjoin.o:    tjoin.c
	$(CC) $(CFLAGS) $(INC) -c tjoin.c -o $@

tbal.o:     $(HDRS) table.h io.h
tbal.o:     tbal.c
	$(CC) $(CFLAGS) $(INC) -c tbal.c -o $@

tnorm.o:    $(HDRS) table.h io.h
tnorm.o:    tnorm.c
	$(CC) $(CFLAGS) $(INC) -c tnorm.c -o $@

t1inn.o:    $(HDRS) io.h attmap.h
t1inn.o:    t1inn.c
	$(CC) $(CFLAGS) $(INC) -c t1inn.c -o $@

xmat.o:     $(UTILDIR)/symtab.h $(UTILDIR)/tfscan.h
xmat.o:     xmat.c
	$(CC) $(CFLAGS) $(INC) -c xmat.c -o $@

uvins.o:    $(HDRS) table.h io.h
uvins.o:    uvins.c
	$(CC) $(CFLAGS) $(INC) -c uvins.c -o $@

skel1.o:    $(HDRS) table.h io.h
skel1.o:    skel1.c
	$(CC) $(CFLAGS) $(INC) -c skel1.c -o $@

skel2.o:    $(HDRS) table.h io.h
skel2.o:    skel2.c
	$(CC) $(CFLAGS) $(INC) -c skel2.c -o $@

#-----------------------------------------------------------------------
# Attribute Set Management
#-----------------------------------------------------------------------
attset1.o:   attset.h $(UTILDIR)/vecops.h
attset1.o:   attset1.c
	$(CC) $(CFLAGS) $(INC) -DAS_RDWR -c attset1.c -o $@

attset2.o:   attset.h $(UTILDIR)/vecops.h
attset2.o:   attset2.c
	$(CC) $(CFLAGS) $(INC) -DAS_RDWR -c attset2.c -o $@

attset3.o:   attset.h $(UTILDIR)/vecops.h
attset3.o:   attset3.c
	$(CC) $(CFLAGS) $(INC) -c attset3.c -o $@

#-----------------------------------------------------------------------
# Attribute Map Management
#-----------------------------------------------------------------------
attmap.o:    attmap.h attset.h
attmap.o:    attmap.c
	$(CC) $(CFLAGS) $(INC) -c attmap.c -o $@

#-----------------------------------------------------------------------
# Table Management
#-----------------------------------------------------------------------
table1.o:   table.h attset.h
table1.o:   table1.c
	$(CC) $(CFLAGS) $(INC) -c table1.c -o $@

table2.o:   table.h attset.h
table2.o:   table2.c
	$(CC) $(CFLAGS) $(INC) -c table2.c -o $@

#-----------------------------------------------------------------------
# Utility Functions for Visualization Programs
#-----------------------------------------------------------------------
tab4vis.o:  tab4vis.h table.h
tab4vis.o:  tab4vis.c
	$(CC) $(CFLAGS) $(INC) -c tab4vis.c -o $@

#-----------------------------------------------------------------------
# Input/Output Utility Functions
#-----------------------------------------------------------------------
io.o:       io.h attset.h $(UTILDIR)/scan.h
io.o:       io.c
	$(CC) $(CFLAGS) $(INC) -c io.c -o $@

io_tab.o:   io.h attset.h table.h
io_tab.o:   io.c
	$(CC) $(CFLAGS) $(INC) -DTAB_RDWR -c io.c -o $@

#-----------------------------------------------------------------------
# External Modules
#-----------------------------------------------------------------------
$(UTILDIR)/vecops.o:
	cd $(UTILDIR); $(MAKE) vecops.o ADDFLAGS=$(ADDFLAGS)
$(UTILDIR)/symtab.o:
	cd $(UTILDIR); $(MAKE) symtab.o ADDFLAGS=$(ADDFLAGS)
$(UTILDIR)/tfscan.o:
	cd $(UTILDIR); $(MAKE) tfscan.o ADDFLAGS=$(ADDFLAGS)
$(UTILDIR)/scform.o:
	cd $(UTILDIR); $(MAKE) scform.o ADDFLAGS=$(ADDFLAGS)
$(UTILDIR)/scan.o:
	cd $(UTILDIR); $(MAKE) scan.o   ADDFLAGS=$(ADDFLAGS)

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
	rm -f *.o *~ *.flc core $(PRGS) skel1 skel2
	cd $(UTILDIR); $(MAKE) clean

localclean:
	rm -f *.o *~ *.flc core $(PRGS) skel1 skel2
