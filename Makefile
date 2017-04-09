SUBDIRS = Apriori HOP KMeans PLSA
SUBDIRS += ScalParC SEMPHY SNP SVM-RFE
SUBDIRS += Utility_Mining/tran_utility Utility_Mining/para_tran_utility
SUBDIRS += ParETI

TAR_FILES = ETI.tar.gz HOP.tar.gz kmeans.tar.gz PLSA.tar.gz
TAR_FILES += ScalParC.tar.gz semphy.tar.gz SNP.tar.gz SVM-RFE.tar.gz
TAR_FILES += utility_mine.tar.gz 
URL = http://cucis.ece.northwestern.edu/projects/DMS/DATASETS/

.PHONY: all $(SUBDIRS) RSEARCH SNP

all: $(SUBDIRS) $(TAR_FILES) RSEARCH SNP

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

RSEARCH:
	cd RSEARCH && ./configure CC=gcc && cd .. 
	$(MAKE) -C $@

SNP:
	cd SNP/pnl.snp/PNL && ./configue CC=gcc CXX=g++ && $(MAKE) && cd ../../.. 
	$(MAKE) -C SNP/pnl.snp/snp

$(TAR_FILES):
	ls datasets || mkdir datasets
	cd datasets
	ls $@ || (wget $(URL)$@ && tar -xvzf $@)
	cd ..
