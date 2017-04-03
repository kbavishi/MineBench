# CXX = icpc -openmp
CXX = g++ -fopenmp

all: snp

PNLD=../PNL

PNLLIBPATH=$(PNLD)/c_pgmtk/src/.libs

INCLUDES = -I$(PNLD)/cxcore/cxcore/include \
           -I$(PNLD)/c_pgmtk/include \
           -I$(PNLD)/c_pgmtk/src/include \
           -I$(PNLD)/c_pgmtk/src/include/cart

CXXFLAGS = -g -O3 -D_SIMD_SCORE -DPNL_STATIC $(INCLUDES)

LIBS = $(PNLLIBPATH)/libpnl.a $(PNLD)/cxcore/cxcore/src/libcxcore.a -lm

snp: testSL_omp.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)

clean:
	rm -f $(OBJS) snp 
