/****************************************************************
File Name: box_fractal.C 
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author, provided that the above copyright notice appear in 
all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#include "global.h"
#include "util.h"
#include "vector.h"
#include "rectangle.h"
#include "grid.h"
#include "box_fractal.h"

int main(int argc, char **argv) {
Rectangle bound;
Grid      *grid;
double    r[MAX_POINTS];
Vector    tmpv;
ifstream boundfile;
ifstream datafile;

if (argc!=3) print_error("Usage:", "fractal boundfile datafile\n");
boundfile.open(argv[1]);
boundfile >> bound; 
for (int i=0; i<MAX_POINTS; i++) {
	boundfile >> r[i];
	grid = new Grid(r[i],bound);
	datafile.open(argv[2]);
	while (!datafile.eof()) {
		datafile >> tmpv;
		if (!datafile.eof())
			grid->Add(tmpv);
		}
	datafile.close();
//	cout << grid->Count() << "\t" << r[i] << "\t" 
     	<< log(grid->Count()) << "\t" << log(r[i]) << endl;
	delete grid;
	}
}


