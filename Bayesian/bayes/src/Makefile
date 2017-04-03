#-----------------------------------------------------------------------
# File    : makefile    (directory: bayes)
# Contents: commands to build naive and full Bayes classifier programs
# Author  : Christian Borgelt
# History : 08.12.1998 file created
#           23.03.1999 storage debugging module added
#           17.04.1999 input/output utility functions added
#           10.11.2000 program corr moved here from table directory
#           26.11.2000 module mvnorm added
#           22.01.2002 adapted to changed table makefile
#           26.04.2003 program bcdb added
#-----------------------------------------------------------------------
include ../../make.common

CFBASE    = -O2 -ansi -Wall -pedantic $(ADDFLAGS)
CFLAGS    = $(CFBASE) -DNDEBUG -O3
CFLAGS    = -g
# CFLAGS    = $(CFBASE) -g
# CFLAGS    = $(CFBASE) -g $(ADDINC) -DSTORAGE
INC       = -I$(UTILDIR) -I$(TABLEDIR)
LIBS      = -lm
# ADDINC    = -I../../misc/src
# ADDOBJ    = storage.o

UTILDIR   = ../../util/src
TABLEDIR  = ../../table/src
HDRS      = $(UTILDIR)/vecops.h   $(UTILDIR)/scan.h \
            $(TABLEDIR)/attset.h  $(TABLEDIR)/table.h
BCHDRS    = $(HDRS) $(UTILDIR)/tfscan.h $(TABLEDIR)/io.h \
            mvnorm.h fbayes.h nbayes.h
OBJS      = $(UTILDIR)/vecops.o   $(UTILDIR)/tfscan.o \
            $(UTILDIR)/scan.o     $(TABLEDIR)/attset1.o \
            $(TABLEDIR)/attset2.o $(TABLEDIR)/attset3.o $(ADDOBJ)
BCI_O     = $(OBJS) $(TABLEDIR)/io_tab.o $(TABLEDIR)/table1.o \
            mvnorm.o fbc_ind.o nbc_ind.o bci.o
BCX_O     = $(OBJS) $(TABLEDIR)/io.o \
            mvn_pars.o fbc_exec.o nbc_exec.o bcx.o
BCDB_O    = $(OBJS) \
            mvn_pars.o fbc_exec.o nbc_exec.o bcdb.o
CORR_O    = $(UTILDIR)/symtab.o $(UTILDIR)/tfscan.o \
            mvnorm.o corr.o $(ADDOBJ)
PRGS      = bci bcx bcdb corr

#-----------------------------------------------------------------------
# Build Programs
#-----------------------------------------------------------------------
all:        $(PRGS)

bci:        $(BCI_O)
	$(CC) $(LDFLAGS) $(BCI_O) $(LIBS) -o $@

bcx:        $(BCX_O)
	$(CC) $(LDFLAGS) $(BCX_O) $(LIBS) -o $@

bcdb:       $(BCDB_O)
	$(CC) $(LDFLAGS) $(BCDB_O) $(LIBS) -o $@

corr:       $(CORR_O)
	$(CC) $(LDFLAGS) $(CORR_O) $(LIBS) -o $@

#-----------------------------------------------------------------------
# Main Programs
#-----------------------------------------------------------------------
bci.o:      $(BCHDRS)
bci.o:      bci.c
	$(CC) $(CFLAGS) $(INC) -c bci.c -o $@

bcx.o:      $(BCHDRS)
bcx.o:      bcx.c
	$(CC) $(CFLAGS) $(INC) -c bcx.c -o $@

bcdb.o:     $(BCHDRS)
bcdb.o:     bcdb.c
	$(CC) $(CFLAGS) $(INC) -DDRAND48 -c bcdb.c -o $@

corr.o:     mvnorm.h $(UTILDIR)/symtab.h $(UTILDIR)/tfscan.h
corr.o:     corr.c
	$(CC) $(CFLAGS) $(INC) -c corr.c -o $@

#-----------------------------------------------------------------------
# Naive Bayes Classifier Management
#-----------------------------------------------------------------------
nbc_ind.o:  nbayes.h $(HDRS)
nbc_ind.o:  nbayes.c
	$(CC) $(CFLAGS) $(INC) -DNBC_INDUCE -c nbayes.c -o $@

nbc_exec.o: nbayes.h $(HDRS)
nbc_exec.o: nbayes.c
	$(CC) $(CFLAGS) $(INC) -DNBC_PARSE -c nbayes.c -o $@

#-----------------------------------------------------------------------
# Full Bayes Classifier Management
#-----------------------------------------------------------------------
fbc_ind.o:  fbayes.h mvnorm.h $(HDRS)
fbc_ind.o:  fbayes.c
	$(CC) $(CFLAGS) $(INC) -DFBC_INDUCE -c fbayes.c -o $@

fbc_exec.o: fbayes.h mvnorm.h $(HDRS)
fbc_exec.o: fbayes.c
	$(CC) $(CFLAGS) $(INC) -DFBC_PARSE -c fbayes.c -o $@

#-----------------------------------------------------------------------
# Multivariate Normal Distribution Management
#-----------------------------------------------------------------------
mvnorm.o:   mvnorm.h $(UTILDIR)/scan.h
mvnorm.o:   mvnorm.c
	$(CC) $(CFLAGS) $(INC) -c mvnorm.c -o $@

mvn_pars.o: mvnorm.h $(UTILDIR)/scan.h
mvn_pars.o: mvnorm.c
	$(CC) $(CFLAGS) $(INC) -DMVN_PARSE -c mvnorm.c -o $@

#-----------------------------------------------------------------------
# Storage Debugging
#-----------------------------------------------------------------------
storage.o:  ../../misc/src/storage.h
storage.o:  ../../misc/src/storage.c
	$(CC) $(CFLAGS) -c ../../misc/src/storage.c -o $@

#-----------------------------------------------------------------------
# External Modules
#-----------------------------------------------------------------------
$(UTILDIR)/vecops.o:
	cd $(UTILDIR);  $(MAKE) vecops.o  ADDFLAGS=$(ADDFLAGS)
$(UTILDIR)/symtab.o:
	cd $(UTILDIR);  $(MAKE) symtab.o  ADDFLAGS=$(ADDFLAGS)
$(UTILDIR)/tfscan.o:
	cd $(UTILDIR);  $(MAKE) tfscan.o  ADDFLAGS=$(ADDFLAGS)
$(UTILDIR)/scan.o:
	cd $(UTILDIR);  $(MAKE) scan.o    ADDFLAGS=$(ADDFLAGS)
$(TABLEDIR)/attset1.o:
	cd $(TABLEDIR); $(MAKE) attset1.o ADDFLAGS=$(ADDFLAGS)
$(TABLEDIR)/attset2.o:
	cd $(TABLEDIR); $(MAKE) attset2.o ADDFLAGS=$(ADDFLAGS)
$(TABLEDIR)/attset3.o:
	cd $(TABLEDIR); $(MAKE) attset3.o ADDFLAGS=$(ADDFLAGS)
$(TABLEDIR)/table1.o:
	cd $(TABLEDIR); $(MAKE) table1.o  ADDFLAGS=$(ADDFLAGS)
$(TABLEDIR)/io.o:
	cd $(TABLEDIR); $(MAKE) io.o      ADDFLAGS=$(ADDFLAGS)
$(TABLEDIR)/io_tab.o:
	cd $(TABLEDIR); $(MAKE) io_tab.o  ADDFLAGS=$(ADDFLAGS)

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
	cd $(UTILDIR);  $(MAKE) clean
	cd $(TABLEDIR); $(MAKE) localclean

localclean:
	rm -f *.o *~ *.flc core $(PRGS)
