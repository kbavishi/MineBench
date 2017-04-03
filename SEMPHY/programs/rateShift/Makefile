#! /usr/local/bin/gmake
#  $Id: Makefile,v 1.1 2006-08-28 09:56:20 osnatzomer Exp $

# use DOUBLEREP=t to activate doubleRep (or setenv DOUBLEREP in the shell)
DOUBLEREP=t

Libsources= 

# LibCsources= cmdline.c
# LibCsources += getopt.c getopt1.c 

EXEC = rateShift
#TEST_EXEC_SUB = split_test splitMap_test bootstrap_test
TEST_EXEC = $(addprefix tests/,$(TEST_EXEC_SUB))
LIB = 

LDFLAGS = -L../../lib 
LDLIBS = -lEvolTree
# LOADLIBES = $(LIB)
#CC=g++

CXX=g++
CC=$(CXX)

#LDFLAGS=  

CPPFLAGS=  -O3 -Wall -Wno-sign-compare -I. -I../../lib -DLOG -ftemplate-depth-25
#CPPFLAGS+= -I/usr/include/g++-v3
#CPPFLAGS+= -DLOG -DLOGCLS -DMEMCHK

# sources
sources= $(Libsources) $(LibCsources) $(addsuffix .cpp,$(EXEC) $(TEST_EXEC))


.PHONY: tests lib test dat debug %.debug DOUBLEREP

ifdef DOUBLEREP
CPPFLAGS+= -DDOUBLEREP
CPPFLAGSDEBUG += -DDOUBLEREP
LDFLAGSDEBUG +=  -DDOUBLEREP
endif

all: lib $(EXEC)

test: all tests
	+cd tests; make -k test

debug: CPPFLAGS = -g -Wall -Wno-sign-compare -I. -I../../lib -DLOG -ftemplate-depth-25
debug: all

pl:
	echo $(LIB)
%.debug: CPPFLAGS = -g -Wall -Wno-sign-compare -I. -I../../lib -DLOG -ftemplate-depth-25
%.debug: %
	@echo "made \""$(*)"\" in debug mode"

lib: $(LIB)

$(LIB): $(Libsources:.cpp=.o) $(LibCsources:.c=.o)
	ar rv $@ $?
	ranlib $@

tags: *.cpp *.h
	etags --language=c++ $^ 

$(EXEC) $(TEST_EXEC): $(LIB) ../../lib/libEvolTree.a         	\
rateShift.o rateShiftOptions.o rateShiftProbs4branch.o branchData.o posData.o
tests: $(TEST_EXEC)

include make.dep

install:
	cd ../fast; make -f Makefile.lib install_do


clean:
	-rm -f $(LIB) $(EXEC) $(TEST_EXEC) *.o

depend makedep: _make.dep
	@mv -f _make.dep make.dep 

_make.dep: $(sources)
	@echo making depend
#	$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $^ | sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@ ; [ -s $@ ] || rm -f $@' 
	@$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $^ > $@' 
_fast:
	cd ../fast;make -f Makefile.lib -k all

fast.% _fast.%:
	cd ../fast;make -f Makefile.lib -k $(*)



# commandline (gengetopts)
%_cmdline.h %_cmdline.c: %.ggo
	$(GENGETOPT)  -i$< -F$(*)_cmdline

cleandat:
	rm 

# DO NOT DELETE
