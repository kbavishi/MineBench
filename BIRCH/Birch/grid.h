/****************************************************************
File Name: grid.h   
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author, provided that the above copyright notice appear in 
all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef GRID_H
#define GRID_H

class Grid {
	double    rsize;
	Rectangle boundary;
	int 	  xsize;
	int 	  ysize;
	int 	  count;
	short     **units;
public:
	Grid(double r, const Rectangle& bound);
	Grid(const Grid& g);
	~Grid();
	void Add(Vector v);
	int  Count();
	};

#endif GRID_H


