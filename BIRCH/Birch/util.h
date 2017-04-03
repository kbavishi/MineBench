/****************************************************************
File Name:   util.h
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef UTIL_H
#define UTIL_H

/* Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 9/95
Permission to use, copy, modify, and distribute this software and
its documentation for any purpose must be granted by the author.
*/

int MaxOne(int x, int y);
int MinOne(int x, int y);
double MaxOne(double x, double y);
double MinOne(double x, double y);

void print_error(char* fnc, char *msg);
void indent(short ind,ofstream &fo);
void indent(short ind,ostream &fo);

#endif

