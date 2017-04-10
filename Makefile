SUBDIRS = Apriori HOP KMeans PLSA
SUBDIRS += ScalParC SEMPHY SVM-RFE
SUBDIRS += Utility_Mining/tran_utility Utility_Mining/para_tran_utility
SUBDIRS += ParETI

TAR_FILES = ETI.tar.gz HOP.tar.gz kmeans.tar.gz PLSA.tar.gz rsearch.tar.gz
TAR_FILES += ScalParC.tar.gz semphy.tar.gz SNP.tar.gz SVM-RFE.tar.gz
#TAR_FILES += utility_mine.tar.gz 
URL = http://cucis.ece.northwestern.edu/projects/DMS/DATASETS/

DATASETS_PATH = /proj/netopt-PG0/datasets

.PHONY: all $(SUBDIRS) RSEARCH

# XXX-kbavishi: Unable to compile SNP/pnl.snp/PNL for some reason. Comment out
# for now.
#.PHONY: all $(SUBDIRS) RSEARCH SNP
#all: $(SUBDIRS) $(TAR_FILES) RSEARCH SNP

all: $(SUBDIRS) $(TAR_FILES) RSEARCH

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

RSEARCH:
	cd RSEARCH && (ls Makefile || ./configure CC=gcc)
	$(MAKE) -C $@

SNP:
	cd SNP/pnl.snp/PNL && (ls Makefile || ./configure CC=gcc CXX=g++)
	$(MAKE) -C SNP/pnl.snp/PNL
	$(MAKE) -C SNP/pnl.snp/snp

$(TAR_FILES):
	mkdir -p datasets
	cd datasets && (ls $@ || ((cp $(DATASETS_PATH)/$@ . || wget $(URL)$@) && tar -xzf $@))
