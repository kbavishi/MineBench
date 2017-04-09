# CXX = icpc -openmp
CXX = g++ -fopenmp

all: snp dataset

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

dataset: 
	ls SNP.tar.gz || \
		(wget http://cucis.ece.northwestern.edu/projects/DMS/DATASETS/SNP.tar.gz && \
		 tar -xvzf SNP.tar.gz)
clean:
	rm -f $(OBJS) snp 
