.KEEP_STATE:

# CC        = icc -openmp
CC          = gcc -fopenmp

DFLAGS      = -D_USE_ATOMIC_PRAGMA_
DFLAGS      =

OPTFLAGS    = -O2
INCFLAGS    =
CFLAGS      = $(OPTFLAGS) $(DFLAGS) $(INCFLAGS)
LDFLAGS     = -lm

H_FILES     = fuzzy_kmeans.h

SRC         = fuzzy_kmeans.c \
              cluster.c \
	      kmeans_clustering.c \
              example.c

OBJ         = $(SRC:%.c=%.o)

.c.o:
	$(CC) $(CFLAGS) -c $<

all: example

example: $(OBJ) $(H_FILES)
	$(CC) $(OPTFLAGS) -o example $(OBJ) $(LDFLAGS)

endian: endian.c
	$(CC) $(OPTFLAGS) -o endian endian.c

clean:
	rm -rf *.o example endian core* .make.state \
	       *.cluster_assign \
               *.cluster_centres group.*
tar:
	tar -cf kmeans.tar $(SRC) $(H_FILES) \
			   Makefile
	gzip kmeans.tar
