# CC = icc -openmp
CC = gcc -fopenmp

CFLAGS = -g $(INCLUDE_DIR) 
LINKFLAGS = 

INCLUDE_DIR = -I. 

LIBS = -lm
TARGET = scalparc
OBJ  = drive.o getdata.o parsort.o parclass.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(LINKFLAGS) $(OBJ) $(LIBS)


.c.o: 
	$(CC) -c $(CFLAGS) $*.c

clean:
	rm -f $(TARGET) $(OBJ)
