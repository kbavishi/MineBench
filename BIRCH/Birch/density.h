/****************************************************************
File Name: density.h   
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/
#ifndef DENSITY_H
#define DENSITY_H

#ifndef QPOINTS
#define QPOINTS 10
#endif QPOINTS

class Density {

public:

Vector *X;
double *FX;
double *PX;

double Rf(Stat *Stats);
void Print_Density(ofstream &fo,Stat *Stats);
void Print_Prob(ofstream &fo,Stat *Stats); 
void Print_Den_Prob(ofstream &fo,Stat *Stats);

Density();
~Density();

void CF_Kernel_Smooth(Stat *Stats);

// Stat* Stats: maintain infomation about CF-tree from Phase1 and Phase2:

// Initialize smoothing parameters
// level 0: naive smoothing, faster, less accurate
// level 1: complex smoothing, slower, more accurate

void CF_Kernel_Start(Stat *Stats,short level);

// given x, calculate density f(x)

void F(Stat* Stats, const Vector& x, double& fx);

// given x, calculate porbability p(t<x)

void P(Stat* Stats, const Vector& x, double& px);

// given x, calculate f(x) and P(x)

void FP(Stat* Stats, const Vector& x, double& fx, double& px);

// given number of points on each dimension,
// x and fx spaces are allocated inside function Fk
// for each x calculate f(x)

void Fk(Stat* Stats, int *k, Vector **x, double **fx);

// given number of points on each dimension,
// a and pa spaces are allocated inside function Pk
// for each x calculate p(t<x)

void Pk(Stat* Stats, int *k, Vector **a, double **px);

// given x, calculate f(x) and P(x)

void FPk(Stat* Stats, int *k, Vector **x, double **fx, double **px);

// assume number of x values are known, 
// x and fx spaces are allocated and assigned correctly,
// for each x calculate f(x)

void Fx(Stat* Stats, int kx, Vector *x, double *fx);

// assume number of a values are known, 
// a and pa spaces are allocated and assigned correctly,
// for each a calculate p(x<a)

void Px(Stat* Stats, int kx, Vector *x, double *px);

// given x, calculate f(x) and P(x)

void FPx(Stat* Stats, int kx, Vector *x, double *fx, double *px);

friend void BirchDensity(Stat **Stats);

};

double H_Oversmooth(Stat *Stats);
double H_Qpoints(Stat *Stats);
double H_Components(Stat *Stats);

void BirchDensity(Stat **Stats);

#endif  DENSITY_H

