#! /usr/local/bin/gmake
#  $Id: Makefile,v 1.2 2006-07-17 10:46:18 ninio Exp $

Libsources= 

# LibCsources= cmdline.c
# LibCsources += getopt.c getopt1.c 

EXEC = readSeq



include ../Makefile.generic

GENGETOPT +=  --unamed-opts=FILES 

# #! /usr/local/bin/gmake
# #  $Id: Makefile,v 1.2 2006-07-17 10:46:18 ninio Exp $

# Libsources= 

# # LibCsources= cmdline.c
# # LibCsources += getopt.c getopt1.c 

# EXEC = readSeq
# #TEST_EXEC_SUB = split_test splitMap_test bootstrap_test
# TEST_EXEC = $(addprefix tests/,$(TEST_EXEC_SUB))
# LIB = 

# LDFLAGS = -L../.. 
# LDLIBS = -lEvolTree
# debug: LDLIBS = -lEvolTreeDebug
# # LOADLIBES = $(LIB)
# #CC=g++
# CXX=g++
# CC=$(CXX)

# #GENGETOPT=/cs/++/phd/ninio/gengetopt-2.11/src/gengetopt
# #GENGETOPT = /opt/local/bin/gengetopt
# GENGETOPT = gengetopt
# .SECONDARY: readSeq_cmdline.c readSeq_cmdline.h readSeq.ggo

# #LDFLAGS=  

# CPPFLAGS=  -O3 -Wall -Wno-sign-compare -I. -I../.. -DLOG -ftemplate-depth-25
# #CPPFLAGS+= -I/usr/include/g++-v3
# #CPPFLAGS+= -DLOG -DLOGCLS -DMEMCHK

# # sources
# sources= $(Libsources) $(LibCsources) $(addsuffix .cpp,$(EXEC) $(TEST_EXEC))


# .PHONY: tests lib test dat debug %.debug

# all: lib $(EXEC)

# test: all tests
# 	+cd tests; make -k test

# debug: CPPFLAGS = -g -Wall -Wno-sign-compare -I. -I../.. -DLOG -ftemplate-depth-25
# debug: all

# pl:
# 	echo $(LIB)
# %.debug: CPPFLAGS = -g -Wall -Wno-sign-compare -I. -I../.. -DLOG -ftemplate-depth-25
# %.debug: %
# 	@echo "made \""$(*)"\" in debug mode"


# lib: $(LIB)

# $(LIB): $(Libsources:.cpp=.o) $(LibCsources:.c=.o)
# 	ar rv $@ $?
# 	ranlib $@

# tags: *.cpp *.h
# 	etags --language=c++ $^ 
# EVOLLIB=../../libEvolTree.a
# debug: EVOLLIB=../../libEvolTreeDebug.a

# $(EXEC): LDLIBS += $(EVOLLIB)
# $(EXEC) $(TEST_EXEC): $(LIB) #$(EVOLLIB)
# tests: $(TEST_EXEC)

# include make.dep

# install:
# 	cd ../fast; make -f Makefile.lib install_do


# clean:
# 	-rm -f $(LIB) $(EXEC) $(TEST_EXEC) *.o

# depend makedep: _make.dep
# 	@mv -f _make.dep make.dep 

# _make.dep: $(sources)
# 	@echo making depend
# #	$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $^ | sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@ ; [ -s $@ ] || rm -f $@' 
# 	@$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $^ > $@' 
# _fast:
# 	cd ../fast;make -f Makefile.lib -k all

# fast.% _fast.%:
# 	cd ../fast;make -f Makefile.lib -k $(*)


# readSeq: readSeq_cmdline.o


# #NJ.ggo: NJ.args ../../evolObjs.args 
# #	cat $^ > $@


# # commandline (gengetopts)
# %_cmdline.h %_cmdline.c: %.ggo
# 	$(GENGETOPT) --unamed-opts=FILES -i$< -F$(*)_cmdline

# cleandat:
# 	rm 

# # DO NOT DELETE
