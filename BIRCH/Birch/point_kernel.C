/****************************************************************
File Name: point_kernel.C   
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
#include "timeutil.h"
#include "vector.h"
#include "rectangle.h"

double Point_Kernel_Density_Effect(Vector &x, Vector &centroid, double H)
{
double tmp=0;

for (short i=0; i<x.dim; i++) 
	tmp-=(x.value[i]-centroid.value[i])*(x.value[i]-centroid.value[i])/
	     (2*H*H);
return exp(tmp)/pow(sqrt(2*PI)*H,x.dim);
}

double Point_Kernel_Prob_Effect(Vector &x, Vector &centroid, double H)
{
return 0.5*(1.0+erf((x.value[0]-centroid.value[0])/(sqrt(2.0)*H)));
}

double Point_Kernel_Rf_Effect(Vector &d1, Vector &d2, double H)
{
double tmp0, tmp1, tmp2;
tmp0=2*H*H;
tmp1=d1.value[0]-d2.value[0];
tmp2=tmp1*tmp1;
tmp1=tmp2/tmp0;
return 1.0/(sqrt(2.0*PI*pow(tmp0,5.0)))*exp(-tmp1/2.0)*((tmp1-3.0)*(tmp1-3.0)-6);
}

main(int argc, char **argv) {

int i,j, nvalues;
double tmp, Rf;

ifstream parafile;
ifstream barfile;
ifstream datafile;
ofstream histfile;


if (argc!=5) print_error("Usage:","point_kernel parafile barfile datafile histfile");

/********** Parameters **************/

parafile.open(argv[1]);

int    dim;
parafile >> dim;
short  WMflag;
parafile >> WMflag;
Vector W;
W.Init(dim);
Vector M;
M.Init(dim);
parafile >> W;
parafile >> M;

double H;
parafile >> H;
int    N;
parafile >> N;

int  *bars; 
bars = new int[dim];
for (i=0; i<dim; i++) parafile >> bars[i];

parafile.close();

/***************** Bar Locations *****************/


barfile.open(argv[2]);

nvalues=1;
for (i=0;i<dim;i++) nvalues*=bars[i];

Vector *x = new Vector[nvalues];
for (i=0; i<nvalues; i++) {
  x[i].Init(dim);
  barfile >> x[i];
  }

double *fx = new double[nvalues];
double *px = new double[nvalues];

for (i=0; i<nvalues; i++) { 
	fx[i]=0.0;
	px[i]=0.0;
	}

barfile.close();

/*********************** Load in data ****************************/

datafile.open(argv[3]);

Vector *data = new Vector[N];
for (i=0; i<N; i++) {
	data[i].Init(dim);
        datafile>>data[i];
     	if (WMflag) data[i].Transform(W,M);
	}

datafile.close();
//cout << "Loading finished\n";

/***************** Use Hos for R(f''), and Plug-in R(f'') for H^* *************/

Rf=0.0;
for (i=0; i<N; i++) {
    if (i%100==0) {//cout << i << endl};
    for (j=0; j<N; j++) {
	   tmp=Point_Kernel_Rf_Effect(data[i],data[j],H);
	   Rf+=tmp/N/N;
	   }
    }

H = pow(1/(2.0*sqrt(PI)*Rf*N),0.2);

/***************** Data Point Effects on Bar Locations  ****************/

for (i=0; i<N; i++) {
     if (i%100==0) {//cout << i << endl}; 
     for (j=0; j<nvalues; j++) {
	   fx[j]+=Point_Kernel_Density_Effect(x[j],data[i],H)/N;
	   px[j]+=Point_Kernel_Prob_Effect(x[j],data[i],H)/N;
	   }
     }

/***************** Histogram *******************/

histfile.open(argv[4]);

histfile << "#Rf = " << Rf << endl;
histfile << "H^* = " << H << endl;

for (i=0; i<nvalues; i++)
	histfile << x[i] << "\t" << fx[i] << "\t" << px[i] << endl;

histfile.close();
}

