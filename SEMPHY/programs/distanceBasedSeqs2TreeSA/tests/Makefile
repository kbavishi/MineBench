CPPFLAGS= -I/usr/include/g++-v3/i386-linux -I/usr/include/g++-v3
CPPFLAGS+=  -O3 -Wall -Wno-sign-compare -I../../lib -DLOG -ftemplate-depth-25
LDFLAGS= -L../../lib
LDLIBS= -lEvolTree -lSEMPHY

CXX=g++
CC=$(CXX)

.PHONY: test tests run debug

all:test
test tests: $(TESTS)

SHTESTS = test.4.posterior.sh

TESTS= $(SHTESTS)

TMPOUT= $(addsuffix .out.tmp,$(TESTS)) 

run:  $(TMPOUT)

debug: CPPFLAGS += -g -Wall -Wno-sign-compare 
debug:
	@echo working in debug mode
%.debug: debug %
	@echo "made \""$(*)"\" in debug mode"



$(addsuffix .out.tmp,$(SHTESTS)): ../distanceBasedSeqs2TreeSA
test: $(addsuffix .test,$(TESTS))

%.test: %.out.tmp %.out.standard
	diff  -I 'the date is'  $^

#cons_test1.out.tmp: $(CPROGS)
#
#cons_test1.out.tmp: lys.tree lys.tree.consraint2 lys.tree.consraint3

%.out.tmp: %
	$(*) >& $@

tests: $(TESTS)


#.INTERMEDIATE: $(TMPOUT)


cleantmp:
	-rm -f  $(TMPOUT)


clean: cleantmp
	-rm -f $(CTEST) 
