/****************************************************************
File Name: global.h   
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author, provided that the above copyright notice appear in 
all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <iostream>
#include <fstream.h>

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif  MIN

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif  MAX

#ifndef ABS
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#endif  ABS

#ifndef SWAP
#define SWAP(x,y) { double _tmp_; _tmp_=(x); (x)=(y); (y)=_tmp_; }
#endif  SWAP

#ifndef PI
#define PI 3.1415926
#endif

#ifndef DATA_SIZE
#define DATA_SIZE (Paras->Dimension*8)
#endif

#ifndef CFENTRY_SIZE
#ifdef  RECTANGLE
#define CFENTRY_SIZE (4+Paras->Dimension*8+8+2*Paras->Dimension*8)
#else
#define CFENTRY_SIZE (4+Paras->Dimension*8+8)
#endif  RECTANGLE
#endif  CFENTRY_SIZE

#ifndef SAMPLE_TIMES
#define SAMPLE_TIMES 5
#endif

#ifndef HISTORY_TIMES
#define HISTORY_TIMES 2
#endif

#ifndef PRECISION_ERROR
#define PRECISION_ERROR 1.0e-30
#endif

// local decision made at each leval:

// X0 euclidian distance
#ifndef D0
#define D0 0
#endif

// X0 manhattan distance
#ifndef D1
#define D1 1
#endif

// average inter-cluster distance 
#ifndef D2
#define D2 2
#endif

// average intra-cluster distance
#ifndef D3
#define D3 3
#endif

// variance increase distance
#ifndef D4
#define D4 4
#endif

#define AVG_DIAMETER 0
#define AVG_RADIUS 1

#define OVERALL_AVG_DIAMETER 0
#define OVERALL_AVG_RADIUS 1

#ifndef ERROR
#define ERROR 1.0e-30 
#endif

#ifndef HUGE_INTEGER
#define HUGE_INTEGER 100000
#endif 

#ifndef  HUGE_DOUBLE
#define HUGE_DOUBLE 99e+99
#endif

#ifndef HUGE
#define HUGE  999999999.9
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif GLOBAL_H








