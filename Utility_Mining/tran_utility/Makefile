# CXX  =  icc
CXX  =  g++

CFLAGS  = -O2 -g  
OBJS	= ListItemset.o
OBJS2   = Database.o Itemset.o HashTree.o
OPTS	= -DBALT -DOPTIMAL 
LIBS = -lm 

TARGET  = utility_mine 

default:	$(TARGET) dataset

clean:
	rm -rf *~ *.o $(TARGET)

utility_mine: utility.cc $(OBJS) $(OBJS2) 
	$(CXX) $(CFLAGS) $(OPTS)\
	-o utility_mine utility.cc $(OBJS) $(OBJS2) $(LIBS)

Database.o: Database.cc Database.h
	$(CXX) $(CFLAGS) $(OPTS) -c Database.cc

Itemset.o: Itemset.cc Itemset.h 
	$(CXX) $(CFLAGS) $(OPTS) -c Itemset.cc

ListItemset.o: ListItemset.cc ListItemset.h
	$(CXX) $(CFLAGS) -c ListItemset.cc

HashTree.o: HashTree.cc HashTree.h
	$(CXX) $(CFLAGS) $(OPTS) -c HashTree.cc

dataset: 
	ls utility_mine.tar.gz || \
		(wget http://cucis.ece.northwestern.edu/projects/DMS/DATASETS/utility_mine.tar.gz && \
		 tar -xvzf utility_mine.tar.gz)

