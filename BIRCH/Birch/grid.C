/****************************************************************
File Name: grid.C  
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


void Grid::Add(Vector v)
{
int i = (int) floor(v.Value(0)/rsize);
int j = (int) floor(v.Value(1)/rsize);
if (units[i][j]==0) {
	units[i][j]=1;
	count++;
	}
}

int Grid::Count()
{
return count;
}

Grid::~Grid()
{
for (int i=0;i<xsize;i++)
	delete [] units[i];
delete [] units;
}

Grid::Grid(const Grid& g)
{
rsize = g.rsize;
boundary = g.boundary;
xsize = g.xsize;
ysize = g.ysize;
count = g.count;
units = g.units;
}

Grid::Grid(double r, const Rectangle& bound) 
{
rsize = r;
boundary = bound;
xsize = (int)floor(bound.Length(0)/r+1);
ysize = (int)floor(bound.Length(1)/r+1);
count = 0;
units = new short*[xsize];
for (int i=0;i<xsize;i++) 
	units[i] = new short[ysize];
for (i=0;i<xsize;i++)
	for (int j=0;j<ysize;j++)
		units[i][j]=0;
}
