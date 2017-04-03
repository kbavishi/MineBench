/****************************************************************
File Name: samples.h   
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef SAMPLES_H
#define SAMPLES_H

#define SAMPLES_SIZE 5

class Stat;

class Sample1 {
public:
int	size;
int	cnt;
int     ptr;
Entry   *CFS;
double  PrevA, PrevB, CurrA, CurrB;

Sample1(int s, Stat *Stats);
~Sample1();
void Take_Sample1(Stat *Stats);
void AvgRRegression(Stat *Stats);
};

class Sample2 {
public:
int     size;
int     ptr;
int	cnt;
int	*NS;
double  *FTS;
double  PrevA,PrevB,CurrA,CurrB;

Sample2(int s);
~Sample2();
void Take_Sample2(Stat *Stats);
void FtDRegression(Stat *Stats);
};

class Sample3 {
public:
int	size;
int 	ptr;
int	cnt;
double 	*logR;
double	*logNR;

Sample3(int s);
~Sample3();
void Take_Sample3(Stat *Stats);
double Regression(const double nr);
};


#endif SAMPLES_H

