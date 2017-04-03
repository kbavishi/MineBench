#! /usr/local/bin/gmake
#  $RCSfile: Makefile,v $ $Revision: 2.3 $ $Date: 2006-06-22 14:51:16 $

Libsources= 

# LibCsources= cmdline.c
# LibCsources += getopt.c getopt1.c 

EXEC = simulateSequnce


include ../Makefile.generic

# #TEST_EXEC_SUB = split_test splitMap_test bootstrap_test
# TEST_EXEC = $(addprefix tests/,$(TEST_EXEC_SUB))
# LIB = 

# libDir=../../lib

# LDFLAGS = -L$(libDir)
# #LDLIBS = -lEvolTree
# #debug: LDLIBS = -lEvolTreeDebug
# # LOADLIBES = $(LIB)
# #CC=g++
# CXX=g++
# CC=$(CXX)

# GENGETOPT = gengetopt

# .SECONDARY: $(addsuffix _cmdline.c,$(EXEC)) $(addsuffix _cmdline.h,$(EXEC))

# #LDFLAGS=  

# CPPFLAGS=  -O3 -Wall -Wno-sign-compare -I. -I$(libDir) -ftemplate-depth-25
# CPPFLAGSDEBUG=  -g -Wall -Wno-sign-compare -I. -I$(libDir) -ftemplate-depth-25
# #CPPFLAGS+= -I/usr/include/g++-v3
# #CPPFLAGS+= -DLOG -DLOGCLS -DMEMCHK

# # sources
# sources= $(Libsources) $(LibCsources) $(addsuffix .cpp,$(EXEC) $(TEST_EXEC))


# .PHONY: tests lib test dat debug %.debug DOUBLEREP

# all: lib $(EXEC)

# test: all tests
# 	+cd tests; make -k test

# ifdef DOUBLEREP
# CPPFLAGS+= -DDOUBLEREP
# endif



# lib: $(LIB)

# $(LIB): $(Libsources:.cpp=.o) $(LibCsources:.c=.o)
# 	ar rv $@ $?
# 	ranlib $@

# vpath % $(libDir)
# tags: *.cpp *.h
# 	etags --language=c++ $^ 
# EVOLLIB=-lEvolTree
# libEvolDebug=-lEvolTreeDebug

# debug: EVOLLIB=$(libEvolDebug)



# #$(EXEC): LDLIBS += $(EVOLLIB)
# #$(EXEC) $(TEST_EXEC): $(LIB) #$(EVOLLIB)
# $(EXEC) $(TEST_EXEC): $(LIB) $(EVOLLIB)
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
# 	+cd ../fast;make -f Makefile.lib -k all

# fast.% _fast.%:
# 	+cd ../fast;make -f Makefile.lib -k $(*)


# $(libEvol):
# 	+cd $(libDir);make -f Makefile all

# $(libEvolDebug):
# 	+cd $(libDir);make -f Makefile debug

# #simulateSequnce: simulateSequnce_cmdline.o
# $(EXEC):  $(addsuffix _cmdline.o,$(EXEC))

# %.ggo: %.args $(libDir)/evolObjs.args 
# 	cat $^ > $@


# # commandline (gengetopts)
# %_cmdline.h %_cmdline.c: %.ggo
# 	$(GENGETOPT)  -i$< -F$(*)_cmdline

# %.dat.q: %.dat
# 	awk 'BEGIN{RS="[\n\r]+";};{print "\" "$$0" \"\r"}' $< >  $@
# #	cat $@

# DAT = cpREV45.dat.q dayhoff.dat.q jones.dat.q mtREV24.dat.q wag.dat.q

# dat: $(DAT)

# cleandat:
# 	rm $(DAT)

# datMatrixHolder.o: $(DAT)

# .PRECIOUS: $(DAT)


# debug: CPPFLAGS = $(CPPFLAGSDEBUG)
# debug: $(addsuffix .debug,$(EXEC))
# #$(addsuffix .debug,$(EXEC)): $(libEvolDebug)
# pl:
# 	echo $(LIB)


# %.debug: CPPFLAGS = -g -Wall -Wno-sign-compare -I. -I../.. -DLOG -ftemplate-depth-25

# %.debug: %.o 

# simulateSequnce.debug: simulateSequnce_cmdline.debug.o


# debug: LDLIBS = -lEvolTreeDebug
# debug: LIB = $(DEBUGLIB)

# %.debug: CPPFLAGS = $(CPPFLAGSDEBUG)
# #%.debug: %
# #	@echo "made \""$(*)"\" in debug mode"


# %.debug.o: %.c
# 	$(CC) -c $(CPPFLAGSDEBUG) $(CFLAGS) $< -o $@

# %.debug.o: %.cpp
# 	$(CXX) -c $(CPPFLAGSDEBUG) $(CXXFLAGS) $< -o $@

# $(DEBUGLIB): $(Libsources:.cpp=.debug.o) $(LibCsources:.c=.debug.o)
# 	ar rv $@ $?
# 	ranlib $@

# # DO NOT DELETE
