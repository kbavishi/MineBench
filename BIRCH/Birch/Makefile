#for rockyroad

include ../make_include

OTHER_INCLUDES = -I../AttrProj

CLUSTER_OBJS=\
	vector.o \
	rectangle.o \
	cfentry.o \
	components.o \
	cftree.o \
	contree.o \
	samples.o \
	path.o \
	buffer.o \
	recyqueue.o \
	parameter.o \
	status.o \
	cutil.o \
	hierarchy.o \
	clarans.o \
	lloyd.o \
	kmeans.o \
	phase1.o \
	phase2.o \
	density.o \
	phase3.o \
	phase4.o \
	main.o \
	timeutil.o \
	util.o \
	vec_utils.o

ASCII2BIN_OBJS = asciiToBinary.o

CLUSTER_SRCS=\
	vector.C \
	rectangle.C \
	cfentry.C \
	components.C \
	cftree.C \
	contree.C \
	samples.C \
	path.C \
	buffer.C \
	recyqueue.C \
	parameter.C \
	status.C \
	cutil.C \
	hierarchy.C \
	clarans.C \
	lloyd.C \
	kmeans.C \
	phase1.C \
	phase2.C \
	density.C \
	phase3.C \
	phase4.C \
	main.C \
	timeutil.C \
	util.C \
	vec_utils.C

ASCII2BIN_SRCS = asciiToBinary.C

# make Birch.o
all:	$(CLUSTER_OBJS) 
	ld -r -o BirchLib.o $(CLUSTER_OBJS)

ascii2Bin: asciiToBinary.o vector.o vec_utils.o
	$(CPLUS) $(CPLUSFLAGS) vector.o vec_utils.o asciiToBinary.o $(LIBDIR) -lm -o ascii2Bin 

summary: summary.o
	$(CPLUS) $(CPLUSFLAGS) vector.o summary.o $(LIBDIR) -lg++ -lm -o summary 

# experiments with point kernel estimation
point_kernel: vector.o rectangle.o util.o timeutil.o point_kernel.o
	$(CPLUS) $(CPLUSFLAGS) vector.o rectangle.o util.o timeutil.o point_kernel.o $(LIBDIR) -lg++ -lm -o point_kernel

# experiments with box fractal dimension 
box_fractal: vector.o rectangle.o util.o grid.o box_fractal.o
	$(CPLUS) $(CPLUSFLAGS) vector.o rectangle.o util.o grid.o box_fractal.o $(LIBDIR) -lg++ -lm -o box_fractal

# experiments with cluster fractal dimension
cluster_fractal: vector.o util.o cluster_fractal.o
	$(CPLUS) $(CPLUSFLAGS) vector.o util.o cluster_fractal.o $(LIBDIR) -lg++ -lm -o cluster_fractal

vector.o: vector.C
	$(CPLUS) $(CPLUSFLAGS) -c vector.C -o vector.o $(INCLUDES) $(OTHER_INCLUDES)
rectangle.o: rectangle.C 
	$(CPLUS) $(CPLUSFLAGS) -c rectangle.C -o rectangle.o $(INCLUDES) $(OTHER_INCLUDES)
parameter.o: parameter.C
	$(CPLUS) $(CPLUSFLAGS) -c parameter.C -o parameter.o $(INCLUDES) $(OTHER_INCLUDES)
util.o: util.C 
	$(CPLUS) $(CPLUSFLAGS) -c util.C -o util.o $(INCLUDES) $(OTHER_INCLUDES)
vec_utils.o: vec_utils.C 
	$(CPLUS) $(CPLUSFLAGS) -c vec_utils.C -o vec_utils.o $(INCLUDES) $(OTHER_INCLUDES)
timeutil.o: timeutil.C 
	$(CPLUS) $(CPLUSFLAGS) -c timeutil.C -o timeutil.o $(INCLUDES) $(OTHER_INCLUDES)
cfentry.o: cfentry.C
	$(CPLUS) $(CPLUSFLAGS) -c cfentry.C -o cfentry.o $(INCLUDES) $(OTHER_INCLUDES)
components.o: components.C
	$(CPLUS) $(CPLUSFLAGS) -c components.C -o components.o $(INCLUDES) $(OTHER_INCLUDES)
cftree.o: cftree.C
	$(CPLUS) $(CPLUSFLAGS) -c cftree.C -o cftree.o $(INCLUDES) $(OTHER_INCLUDES)
contree.o: contree.C
	$(CPLUS) $(CPLUSFLAGS) -c contree.C -o contree.o $(INCLUDES) $(OTHER_INCLUDES)
path.o: path.C
	$(CPLUS) $(CPLUSFLAGS) -c path.C -o path.o $(INCLUDES) $(OTHER_INCLUDES)
buffer.o: buffer.C
	$(CPLUS) $(CPLUSFLAGS) -c buffer.C -o buffer.o $(INCLUDES) $(OTHER_INCLUDES)
recyqueue.o: recyqueue.C
	$(CPLUS) $(CPLUSFLAGS) -c recyqueue.C -o recyqueue.o $(INCLUDES) $(OTHER_INCLUDES)
status.o: status.C
	$(CPLUS) $(CPLUSFLAGS) -c status.C -o status.o $(INCLUDES) $(OTHER_INCLUDES)
cutil.o:  cutil.C
	$(CPLUS) $(CPLUSFLAGS) -c cutil.C -o cutil.o $(INCLUDES) $(OTHER_INCLUDES)
hierarchy.o: hierarchy.C
	$(CPLUS) $(CPLUSFLAGS) -c hierarchy.C -o hierarchy.o $(INCLUDES) $(OTHER_INCLUDES)
clarans.o: clarans.C
	$(CPLUS) $(CPLUSFLAGS) -c clarans.C -o clarans.o $(INCLUDES) $(OTHER_INCLUDES)
kmeans.o: kmeans.C
	$(CPLUS) $(CPLUSFLAGS) -c kmeans.C -o kmeans.o $(INCLUDES) $(OTHER_INCLUDES)
lloyd.o: lloyd.C
	$(CPLUS) $(CPLUSFLAGS) -c lloyd.C -o lloyd.o $(INCLUDES) $(OTHER_INCLUDES)
phase1.o:  phase1.C
	$(CPLUS) $(CPLUSFLAGS) -c phase1.C -o phase1.o $(INCLUDES) $(OTHER_INCLUDES)
incrPhase1.o:  incrPhase1.C
	$(CPLUS) $(CPLUSFLAGS) -c incrPhase1.C -o incrPhase1.o $(INCLUDES) $(OTHER_INCLUDES)
phase2.o:  phase2.C
	$(CPLUS) $(CPLUSFLAGS) -c phase2.C -o phase2.o $(INCLUDES) $(OTHER_INCLUDES)
density.o:  density.C
	$(CPLUS) $(CPLUSFLAGS) -c density.C -o density.o $(INCLUDES) $(OTHER_INCLUDES)
phase3.o:  phase3.C
	$(CPLUS) $(CPLUSFLAGS) -c phase3.C -o phase3.o $(INCLUDES) $(OTHER_INCLUDES)
phase4.o:  phase4.C
	$(CPLUS) $(CPLUSFLAGS) -c phase4.C -o phase4.o $(INCLUDES) $(OTHER_INCLUDES)
main.o: main.C
	$(CPLUS) $(CPLUSFLAGS) -c main.C -o main.o $(INCLUDES) $(OTHER_INCLUDES)
incrMain.o: incrMain.C
	$(CPLUS) $(CPLUSFLAGS) -c incrMain.C -o incrMain.o $(INCLUDES) $(OTHER_INCLUDES)
asciiToBinary.o: asciiToBinary.C
	$(CPLUS) $(CPLUSFLAGS) -c asciiToBinary.C -o asciiToBinary.o $(INCLUDES) $(OTHER_INCLUDES)
samples.o: samples.C
	$(CPLUS) $(CPLUSFLAGS) -c samples.C -o samples.o $(INCLUDES) $(OTHER_INCLUDES)
rand.o: rand.C 
	$(CPLUS) $(CPLUSFLAGS) -c rand.C -o rand.o $(INCLUDES) $(OTHER_INCLUDES)

point_kernel.o: point_kernel.C
	$(CPLUS) $(CPLUSFLAGS) -c point_kernel.C -o point_kernel.o $(INCLUDES)

grid.o: grid.C
	$(CPLUS) $(CPLUSFLAGS) -c grid.C -o grid.o $(INCLUDES)

box_fractal.o: box_fractal.C
	$(CPLUS) $(CPLUSFLAGS) -c box_fractal.C -o box_fractal.o $(INCLUDES)

cluster_fractal.o: cluster_fractal.C
	$(CPLUS) $(CPLUSFLAGS) -c cluster_fractal.C -o cluster_fractal.o $(INCLUDES)

clean:
	/bin/rm -f $(CLUSTER_OBJS) $(ASCII2BIN_OBJS) BirchLib.o

depend:
	$(MAKEDEPEND) -- $(CPLUSFLAGS) -- $(CLUSTER_SRCS) grid.C box_fractal.C cluster_fractal.C $(INCLUDES) $(OTHER_INCLUDES) >! .depend

include .depend

