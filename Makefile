SUBDIRS = Apriori HOP KMeans PLSA RSEARCH
SUBDIRS += ScalParC SEMPHY SNP SVM-RFE Utility_Mining ParETI

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
