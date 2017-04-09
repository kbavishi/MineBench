.PHONY: all lib semphy programs clean install

all: lib programs dataset

debug: lib.debug

%: lib.% programs.%
	echo $@

lib: lib.lib

programs: programs.all

programs.all: lib
programs.debug: lib.debug

semphy: programs.semphy

install: programs.install

programs.install programs.all semphy: lib

dataset: 
	ls semphy.tar.gz || \
		(wget http://cucis.ece.northwestern.edu/projects/DMS/DATASETS/semphy.tar.gz && \
		 tar -xvzf semphy.tar.gz)

clean: lib.clean programs.clean

lib.%:
	+cd lib;make $(*)

programs.%:
	+cd programs;make $(*)

tags: lib/*.cpp lib/*.h programs/*/*.h programs/*/*.cpp
	etags --language=c++ $^ 

