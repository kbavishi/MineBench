#!/usr/bin/python

# splits the file into multiple pieces (useful for doing parallel processing)

import sys
import os

# get the arguments
filename = sys.argv[2]
splits = int(sys.argv[1])

# create the files for writing
files = []
for i in range(splits):
    files.append(open(filename + "_" + str(i), "w"))


# open up the input file
lineNum = 0;
inputfile = open(filename)
line = inputfile.readline()
while (line != ""):
    files[lineNum % splits].write(line)
    lineNum = lineNum + 1
    line = inputfile.readline()

for file in files:
    file.close()

inputfile.close()
		
cmd = "mpirun -np " + sys.argv[1] + " -machinefile machines eti " + filename + " " + sys.argv[3] + " " + sys.argv[4] + " " + sys.argv[5];
print cmd;
os.system(cmd);

