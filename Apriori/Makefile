CXX     = g++
OMPFLAG = -fopenmp

OPTS	= -g -O2 -DBALT 
CFLAGS  = $(OPTS) $(OMPFLAG)
LIBS    = -lm 

HEADERS = Itemset.h \
          ListItemset.h \
          HashTree.h \
          Database.h \
          pardhp.h

SRCS	= ListItemset.C \
          Database.C \
          Itemset.C \
          HashTree.C \
          omp_apriori.C

OBJS    = $(SRCS:.C=.o)

TARGET  = omp_apriori

default: $(TARGET)

.C.o:
	$(CXX) $(CFLAGS) -c $<

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

omp_apriori.o: omp_apriori.C $(HEADERS)

Database.o: Database.C Database.h

Itemset.o: Itemset.C Itemset.h 

ListItemset.o: ListItemset.C ListItemset.h

HashTree.o: HashTree.C HashTree.h

# offsets.c is a program to generate the offset file
offsets: offsets.c
	$(CXX) -g $< -o $@

clean:
	rm -rf *~ *.o $(TARGET) offsets

