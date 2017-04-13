SUBDIRS = Apriori Bayesian BIRCH ECLAT ECLAT/util HOP
SUBDIRS += KMeans PLSA ScalParC SEMPHY SVM-RFE
#SUBDIRS += Utility_Mining/tran_utility Utility_Mining/para_tran_utility
SUBDIRS += ParETI

TAR_FILES = APR birch Bayesian ETI HOP kmeans PLSA rsearch
TAR_FILES += ScalParC semphy SNP SVM-RFE
#TAR_FILES += utility_mine

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
	ls $(DATASETS_PATH)/$@.tar.gz || (cd $(DATASETS_PATH) && wget $(URL)$@.tar.gz)
	ls datasets/$@ || tar -xzf $(DATASETS_PATH)/$@.tar.gz -C datasets/
