# CC = icc -openmp
CC = gcc -fopenmp

CFLAGS = -g $(INCLUDE_DIR) 
LINKFLAGS = 

INCLUDE_DIR = -I. 

LIBS = -lm
TARGET = scalparc
OBJ  = drive.o getdata.o parsort.o parclass.o

all: $(TARGET) dataset

$(TARGET): $(OBJ)
	$(CC) -o $@ $(LINKFLAGS) $(OBJ) $(LIBS)


.c.o: 
	$(CC) -c $(CFLAGS) $*.c

dataset: 
	ls ScalParC.tar.gz || \
		(wget http://cucis.ece.northwestern.edu/projects/DMS/DATASETS/ScalParC.tar.gz && \
		 tar -xvzf ScalParC.tar.gz)
clean:
	rm -f $(TARGET) $(OBJ)
