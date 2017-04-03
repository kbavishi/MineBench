CXX  = g++

CFLAGS  = -O3 -Wno-non-template-friend
HEADER  = Array.h Itemset.h Lists.h Eqclass.h HashTable.h \
	Graph.h extl2.h partition.h memman.h GArray.h Util.h \
	assoc.h calcdb.h
OBJS	= Itemset.o Array.o Eqclass.o HashTable.o \
	Graph.o extl2.o partition.o memman.o GArray.o Util.o calcdb.o
LIBS = -lm -lc
TARGET  = eclat

default:	$(TARGET) dataset

dataset: 
	ls ECLAT.tar.gz || \
		(wget http://cucis.ece.northwestern.edu/projects/DMS/DATASETS/ECLAT.tar.gz && \
		 tar -xvzf ECLAT.tar.gz)


clean:
	rm -rf *~ *.o $(TARGET)

eclat: eclat.cc $(OBJS) $(HEADER) stats.h
	$(CXX) $(CFLAGS) -o eclat eclat.cc $(OBJS) $(LIBS)
#	strip eclat

assocFB: assocFB.cc $(OBJS) $(HEADER) stats.h
	$(CXX) $(CFLAGS) -o assocFB assocFB.cc $(OBJS) $(LIBS)
	strip assocFB

maximal: maximal.cc Itemset.o Array.o Lists.h 
	$(CXX) $(CFLAGS) -o maximal maximal.cc Itemset.o Array.o $(LIBS)

.SUFFIXES: .o .cc .c   

.cc.o: 
	$(CXX) $(CFLAGS) -c $<  

# dependencies
# $(OBJS): $(HFILES)
Database.o: Database.h
Itemset.o: Itemset.h
Array.o: Array.h
Eqclass.o: Eqclass.h
HashTable.o: HashTable.h
extl2.o: extl2.h
partition.o: partition.h
memman.o: memman.h
GArray.o: GArray.h
Graph.o: Graph.h
Util.o: Util.h
calcdb.o: calcdb.h




