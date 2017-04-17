SUBDIRS = Apriori BIRCH ECLAT ECLAT/util HOP
SUBDIRS += KMeans ScalParC

TAR_FILES = APR.tar.gz birch.tar.gz HOP.tar.gz kmeans.tar.gz
TAR_FILES += ScalParC.tar.gz
#TAR_FILES += utility_mine

URL = http://cucis.ece.northwestern.edu/projects/DMS/DATASETS/

DATASETS_PATH = /proj/netopt-PG0/datasets

.PHONY: all $(SUBDIRS) RSEARCH

# XXX-kbavishi: Unable to compile SNP/pnl.snp/PNL for some reason. Comment out
# for now.
#.PHONY: all $(SUBDIRS) RSEARCH SNP
#all: $(SUBDIRS) $(TAR_FILES) RSEARCH SNP

all: $(SUBDIRS) $(TAR_FILES)

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
	ls $(DATASETS_PATH)/$@ || (cd $(DATASETS_PATH) && wget $(URL)$@)
	ls datasets/$(subst .tar.gz,,$@) || tar -xzf $(DATASETS_PATH)/$@ -C datasets/
