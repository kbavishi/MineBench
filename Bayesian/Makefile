SUBDIRS = bayes \
          table \
          math \
          matrix \
          bayes

all:
	set -e; for i in $(SUBDIRS); do ( cd $$i/src && $(MAKE) $(MFLAGS) all ; ) ; done

clean:
	set -e; for i in $(SUBDIRS); do ( cd $$i/src && $(MAKE) $(MFLAGS) clean ; ) ; done

