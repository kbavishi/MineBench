/****************************************************************
File Name:   util.C
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#include "global.h"
#include "util.h"

int MinOne(int x, int y)
{ if (x<y) return x;
  else return y;
}

int MaxOne(int x, int y)
{ if (x>y) return x;
  else return y;
}

double MinOne(double x, double y)
{ if (x<y) return x;
  else return y;
}

double MaxOne(double x, double y)
{ if (x>y) return x;
  else return y;
}

void indent(short ind, ostream &fo)
{ for (short i=0; i<ind; i++) 
	fo << ' '; 
}

void indent(short ind, ofstream &fo)
{ for (short i=0; i<ind; i++) 
	fo << ' '; 
}

void print_error(char *fnc, char *msg) {
        printf("Fatal Error: in function %s: %s\n",fnc,msg);
        exit(1);
}


