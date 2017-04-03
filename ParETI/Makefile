# makefile for ParETI implementation
# Blayne Field
# 31 May 2006

MPICXX  = mpicxx

CFLAGS  = -Wall -O3
LDFLAGS =
SOURCES = ETIMiner.cpp DataSet.cpp SparseItemset.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = pareti
TARBALL_NAME = ParETI.tar.gz
DEFAULT_PARAMETERS = #hugetestdata10 22500 0.1 0.0 

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(MPICXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(MPICXX) $(CFLAGS) -c $<

dist: $(SOURCES)
	rm -f $(TARBALL_NAME) $(TARBALL_NAME).gz
	rm -rf ParETI
	mkdir ParETI
	cp *.cpp *.h runeti.py Makefile README ParETI
	tar -zcvf $(TARBALL_NAME) ParETI
	rm -rf ParETI

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(DEFAULT_PARAMETERS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

