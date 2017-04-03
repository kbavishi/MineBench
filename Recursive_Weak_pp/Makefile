# makefile for rwpp implementation
# Blayne Field
# 31 May 2006

# CXX = mpicxx -DUSE_MPI
CXX = g++

CFLAGS = -Wall -O3# -g -pg
LDFLAGS =  #-pg
SOURCES = ETIMiner.cpp DataSet.cpp SparseItemset.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = rwpp
TARBALL_NAME = ParETI.tar.gz
DEFAULT_PARAMETERS = #hugetestdata10 22500 0.1 0.0 

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) $(CFLAGS) -c $<

dist: $(SOURCES)
	rm -f $(TARBALL_NAME) $(TARBALL_NAME).gz
	rm -rf ParETI
	mkdir ParETI
	cp *.cpp *.h Makefile README ParETI
	tar -zcvf $(TARBALL_NAME) ParETI
	rm -rf ParETI

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(DEFAULT_PARAMETERS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

