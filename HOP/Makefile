# Edit the following variables as needed
CXX     = g++
OMPFLAG = -fopenmp

CFLAGS = -O -g -Wno-write-strings $(OMPFLAG)
LIB    = -lm

SRCS   = para_hop.c      \
         para_hop_kd.c   \
         hop_smooth.c    \
         para_median.c   \
         hop_regroup.c   \
         hop_slice.c

OBJS   = $(SRCS:%.c=%.o)

PROGS  = para_hop

.c.o:
	$(CXX) $(CFLAGS) -c $<

all: $(PROGS)

para_hop : $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $(OBJS) $(LIB)

para_hop.o :    para_hop.c

para_hop_kd.o : para_hop_kd.c

hop_smooth.o : hop_smooth.c smooth.h

para_median.o : para_median.c

hop_regroup.o : hop_regroup.c

hop_slice.o   :hop_slice.c

clean:
	rm -f core $(OBJS) $(PROGS)

.SUFFIXES:.o.c

