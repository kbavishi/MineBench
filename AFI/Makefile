# makefile for AFI implementation
# Blayne Field
# 6 June 2006

CXX          = g++
CFLAGS       = -O3 -Wall
LDFLAGS      = #-pg
SOURCES      = AFIMiner.cpp \
               DataSet.cpp \
               ItemSetTree.cpp \
               Bitmap.cpp

OBJECTS      = $(SOURCES:.cpp=.o)
EXECUTABLE   = afi

TARBALL_NAME       = AFI.tar.gz
DEFAULT_PARAMETERS = 

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) $(CFLAGS) -c $<

dist: $(SOURCES)
	rm -f $(TARBALL_NAME) $(TARBALL_NAME).gz
	rm -rf AFI
	mkdir AFI
	cp *.cpp *.h makefile $(EXECUTABLE) AFI
	tar -zcvf $(TARBALL_NAME) AFI
	rm -rf AFI

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(DEFAULT_PARAMETERS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

