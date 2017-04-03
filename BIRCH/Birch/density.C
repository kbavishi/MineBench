/****************************************************************
File Name: density.C   
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#include <assert.h>
#include "global.h"
#include "util.h"
#include "vector.h"
#include "rectangle.h"
#include "parameter.h"
#include "cfentry.h"
#include "status.h"
#include "cftree.h"
#include "cutil.h"
#include "density.h"

extern Para *Paras;

Density::Density() {
X = NULL;
FX = NULL;
PX = NULL;
}

Density::~Density() {
if (X!=NULL) delete [] X;
if (FX!=NULL) delete [] FX;
if (PX!=NULL) delete [] PX;
}

void Density::Print_Density(ofstream &fo,Stat *Stats) {
int i, nvalues=1;
for (i=0; i<Stats->Dimension; i++)
	nvalues*=Stats->Bars[i];

for (i=0; i<nvalues; i++)
	fo<<X[i]<<"\t"<<FX[i]<<endl;
}

void Density::Print_Prob(ofstream &fo,Stat *Stats) {
int i, nvalues=1;
for (i=0; i<Stats->Dimension; i++)
	nvalues*=Stats->Bars[i];

for (i=0; i<nvalues; i++) 
	fo<< X[i]<<"\t"<<PX[i]<<endl;
}

void Density::Print_Den_Prob(ofstream &fo,Stat *Stats) {
int i, nvalues=1;
for (i=0; i<Stats->Dimension; i++)
	nvalues*=Stats->Bars[i];

for (i=0; i<nvalues; i++)
	fo<<X[i]<<"\t"<<FX[i]<<"\t"<<PX[i]<<endl;
}

// calculate the h^* by double-scanning all CF-kernels for R(f^'')

double Density::Rf(Stat* Stats)
{
int    i,j,ndata;
double tmp, tmpRf=0.0;
Leaf   *tmpleaf1, *tmpleaf2;
Entry  tmpent1, tmpent2;
tmpent1.Init(Stats->Dimension);
tmpent2.Init(Stats->Dimension);

ndata=Stats->NewRoot->N();

i=0;
tmpleaf1=Stats->NewLeafHead;
while (Stats->NextEntryFromLeafHead(i,tmpent1,&tmpleaf1)!=FALSE) {
  j=0;
  tmpleaf2=Stats->NewLeafHead;
  while (Stats->NextEntryFromLeafHead(j,tmpent2,&tmpleaf2)!=FALSE) {
	    switch (Stats->CFDistr) {
	    case 0: tmp=Unif_Kernel_Rf_Effect(tmpent1,tmpent2,Stats->H);	
		    break;
	    case 1: tmp=Norm_Kernel_Rf_Effect(tmpent1,tmpent2,Stats->H);	
		    break;
	    }
	    tmpRf+=tmp*tmpent1.n/ndata*tmpent2.n/ndata;
	}
  }
return tmpRf;
}

// H is default as Hos=[(243*R(K))/(35*K2*n)]^[1/5]*r,
// where R(K) = 1/(2*sqrt(PI)), K2=1 for normal kernel
double H_Oversmooth(Stat *Stats)
{
return pow((243/(2.0*sqrt(PI)))/(35*Stats->NewRoot->N()),0.2)*
       sqrt(Stats->NewRoot->Radius());
}

static double AvgQpointsDist(Leaf *tmpleaf, int i)
{
int    n=tmpleaf->entry[i].n-1;
double d=n*sqrt(tmpleaf->entry[i].Diameter());

int    tmpj, Minj;
double tmpD, MinD;

short  done=FALSE;

short  *flag=new short[tmpleaf->actsize];
for (tmpj=0; tmpj<tmpleaf->actsize; tmpj++) flag[tmpj]=TRUE;

flag[i]=FALSE;
while (done==FALSE) {

done=TRUE;
MinD=HUGE_DOUBLE;
for (tmpj=0; tmpj<tmpleaf->actsize; tmpj++) 
	if (flag[tmpj]==TRUE) { 
		done=FALSE;
		tmpD=distance(D2,tmpleaf->entry[i],tmpleaf->entry[tmpj]);
		if (MinD>tmpD) 
			{MinD=tmpD; Minj=tmpj;}
		}

if (done==TRUE) {
	delete [] flag;
	return d/n;
	}

if (n+tmpleaf->entry[Minj].n>=QPOINTS) {
	d+=(QPOINTS-n)*sqrt(MinD);
	n=QPOINTS;
	delete [] flag;
	return d/n;
	}

d+=tmpleaf->entry[Minj].n*sqrt(MinD);
n+=tmpleaf->entry[Minj].n;
flag[Minj]=FALSE;
}
}

double H_Qpoints(Stat *Stats)
{
int    n=0;
double d=0.0;
int    i1=0,i2=0;
Leaf   *tmpleaf1=Stats->NewLeafHead;
Leaf   *tmpleaf2=Stats->NewLeafHead;
Entry  tmpent;
tmpent.Init(Stats->Dimension);

while (Stats->NextEntryFromLeafHead(i2,tmpent,&tmpleaf2)!=FALSE) {
	if (tmpent.n>QPOINTS) {
		n+=tmpent.n;
		d+=tmpent.n*sqrt(tmpent.Diameter());
		}
	else {  n+=tmpent.n;
		d+=tmpent.n*AvgQpointsDist(tmpleaf1,i1);
		}
	i1=i2;
	tmpleaf1=tmpleaf2;
	}
return d/n;
}

double H_Components(Stat *Stats)
{
}

void Density::CF_Kernel_Start(Stat *Stats,short level)
{
double tmpRf;

// set initial H:
if (Stats->H>0) {
      // H is given by the user as a positive real number,
//      Paras->logfile<<"H0="<<Stats->H<<endl;
      } 
else {
      // approach 1:
      // Stats->H=H_Oversmooth(Stats);
      // Paras->logfile<<"Hos="<<Stats->H<<endl;

      // approach 2:
      Stats->H=H_Qpoints(Stats);
//      Paras->logfile<<"Hqp="<<Stats->H<<endl;

      // approach 3:
      // Stats->H=H_Components(Stats);
      // Paras->logfile<<"Hco="<<Stats->H<<endl;
      }

// verify H: plug in R(f'')
if (level==1) {
     tmpRf=Rf(Stats);
//     Paras->logfile<<"Rf="<<tmpRf<<endl;

     Stats->H=pow(1.0/(2.0*sqrt(PI)*tmpRf*Stats->NewRoot->N()),0.2);
//     Paras->logfile<<"H^*="<<Stats->H<<endl;
     }
}

// calculate the value by scanning all CF-kernels
void Density::F(Stat* Stats, const Vector& x, double &fx)
{
int 	 i,ndata;
double   tmp;
Leaf	 *tmpleaf1;
Entry    tmpent1;
tmpent1.Init(Stats->Dimension);

ndata=Stats->NewRoot->N();

i=0;
tmpleaf1=Stats->NewLeafHead;
while (Stats->NextEntryFromLeafHead(i,tmpent1,&tmpleaf1)!=FALSE) {
	switch (Stats->CFDistr) {
	case 0: tmp=tmpent1.Unif_Kernel_Density_Effect(x,Stats->H);
		break;
	case 1: tmp=tmpent1.Norm_Kernel_Density_Effect(x,Stats->H);
		break;
		}
	fx=tmp*tmpent1.n/ndata;
	}
}

// calculate the value by scanning all CF-kernels
void Density::P(Stat* Stats, const Vector& x, double &px)
{
int 	 i,ndata;
double   tmp;
Leaf	 *tmpleaf1;
Entry    tmpent1;
tmpent1.Init(Stats->Dimension);

ndata=Stats->NewRoot->N();

i=0;
tmpleaf1=Stats->NewLeafHead;
while (Stats->NextEntryFromLeafHead(i,tmpent1,&tmpleaf1)!=FALSE) {
	switch (Stats->CFDistr) {
	case 0: tmp=tmpent1.Unif_Kernel_Prob_Effect(x,Stats->H);
		break;
	case 1: tmp=tmpent1.Norm_Kernel_Prob_Effect(x,Stats->H);
		break;
		}
	px=tmp*tmpent1.n/ndata;
	}
}

// calculate the value by scanning all CF-kernels
void Density::FP(Stat* Stats, const Vector& x, double &fx, double &px)
{
int 	 i,ndata;
double   tmp1,tmp2;
Leaf	 *tmpleaf1;
Entry    tmpent1;
tmpent1.Init(Stats->Dimension);

ndata=Stats->NewRoot->N();

i=0;
tmpleaf1=Stats->NewLeafHead;
while (Stats->NextEntryFromLeafHead(i,tmpent1,&tmpleaf1)!=FALSE) {
	switch (Stats->CFDistr) {
	case 0: tmp1=tmpent1.Unif_Kernel_Density_Effect(x,Stats->H);
		tmp2=tmpent1.Unif_Kernel_Prob_Effect(x,Stats->H);
		break;
	case 1: tmp1=tmpent1.Norm_Kernel_Density_Effect(x,Stats->H);
		tmp2=tmpent1.Norm_Kernel_Prob_Effect(x,Stats->H);
		break;
		}
	fx=tmp1*tmpent1.n/ndata;
	px=tmp2*tmpent1.n/ndata;
	}
}

void Density::Fk(Stat *Stats, int *k, Vector **x, double **fx)
{
int     i, j, k1, k2, ndata, nvalues;
double  tmp;
Leaf    *tmpleaf1;
Entry   tmpent1;
tmpent1.Init(Stats->Dimension);

ndata=Stats->NewRoot->N();

// allocate *x and *fx
nvalues=1;
for (i=0;i<Stats->Dimension;i++)
	nvalues*=k[i];

(*x) = new Vector[nvalues];
for (i=0;i<nvalues;i++) {
	(*x)[i].Init(Stats->Dimension);
	k1=i;
	for (j=0;j<Stats->Dimension;j++) {
		k1=k1*k[j]/nvalues;
		k2=k1*k[j]%nvalues;
		(*x)[i].value[j]=Stats->Ranges.low[j]+
			k1*(Stats->Ranges.high[j]-Stats->Ranges.low[j])/k[j];
		k1=k2;
		}
	}
	
*fx = new double[nvalues];
for (i=0; i<nvalues;i++) 
	(*fx)[i]=0.0;

// calculate the values by scanning all CF-kernels
i=0;
tmpleaf1=Stats->NewLeafHead;
while (Stats->NextEntryFromLeafHead(i,tmpent1,&tmpleaf1)!=FALSE) {
   for (j=0; j<nvalues; j++) {
	switch (Stats->CFDistr) {
	case 0: tmp=tmpent1.Unif_Kernel_Density_Effect((*x)[j],Stats->H);
		break;
	case 1: tmp=tmpent1.Norm_Kernel_Density_Effect((*x)[j],Stats->H);
		break;
	}
	(*fx)[j]+=tmp*tmpent1.n/ndata;
	}
   }
}

void Density::Pk(Stat* Stats, int *k, Vector** x, double **px)
{
int     i, j, k1, k2, ndata, nvalues;
double  tmp;
Leaf    *tmpleaf1;
Entry   tmpent1;
tmpent1.Init(Stats->Dimension);

ndata=Stats->NewRoot->N();

// allocate *x and *px

nvalues=1;
for (i=0;i<Stats->Dimension;i++)
	nvalues*=k[i];

(*x) = new Vector[nvalues];
for (i=0;i<nvalues;i++) {
	(*x)[i].Init(Stats->Dimension);
	k1=i;
	for (j=0;j<Stats->Dimension;j++) {
		k1=k1*k[j]/nvalues;
		k2=k1*k[j]%nvalues;
		(*x)[i].value[j]=Stats->Ranges.low[j]+
			k1*(Stats->Ranges.high[j]-Stats->Ranges.low[j])/k[j];
		k1=k2;
		}
	}
	
(*px) = new double[nvalues];
for (i=0; i<nvalues;i++) 
	(*px)[i]=0.0;

// calculate the values by scanning all CF-kernels
i=0;
tmpleaf1=Stats->NewLeafHead;
while (Stats->NextEntryFromLeafHead(i,tmpent1,&tmpleaf1)!=FALSE) {
   for (j=0; j<nvalues; j++) {
	switch (Stats->CFDistr) {
	case 0: tmp=tmpent1.Unif_Kernel_Prob_Effect((*x)[j],Stats->H);
		break;
	case 1: tmp=tmpent1.Norm_Kernel_Prob_Effect((*x)[j],Stats->H);
		break;
	}
	(*px)[j]+=tmp*tmpent1.n/ndata;
	}
   }
}

void Density::FPk(Stat *Stats, int *k, Vector **x, double **fx, double **px)
{
int     i, j, k1, k2, ndata, nvalues;
double  tmp1, tmp2;
Leaf    *tmpleaf1;
Entry   tmpent1;
tmpent1.Init(Stats->Dimension);

ndata=Stats->NewRoot->N();

// allocate *x and *fx
nvalues=1;
for (i=0;i<Stats->Dimension;i++)
	nvalues*=k[i];

(*x) = new Vector[nvalues];
for (i=0;i<nvalues;i++) {
	(*x)[i].Init(Stats->Dimension);
	k1=i;
	for (j=0;j<Stats->Dimension;j++) {
		k1=k1*k[j]/nvalues;
		k2=k1*k[j]%nvalues;
		(*x)[i].value[j]=Stats->Ranges.low[j]+
			k1*(Stats->Ranges.high[j]-Stats->Ranges.low[j])/k[j];
		k1=k2;
		}
	}
	
*fx = new double[nvalues];
*px = new double[nvalues];
for (i=0; i<nvalues;i++) {
	(*fx)[i]=0.0;
	(*px)[i]=0.0;
	}

// calculate the values by scanning all CF-kernels
i=0;
tmpleaf1=Stats->NewLeafHead;
while (Stats->NextEntryFromLeafHead(i,tmpent1,&tmpleaf1)!=FALSE) {
   for (j=0; j<nvalues; j++) {
	switch (Stats->CFDistr) {
	case 0: tmp1=tmpent1.Unif_Kernel_Density_Effect((*x)[j],Stats->H);
	        tmp2=tmpent1.Unif_Kernel_Prob_Effect((*x)[j],Stats->H);
		break;
	case 1: tmp1=tmpent1.Norm_Kernel_Density_Effect((*x)[j],Stats->H);
	        tmp2=tmpent1.Norm_Kernel_Prob_Effect((*x)[j],Stats->H);
		break;
	}
	(*fx)[j]+=tmp1*tmpent1.n/ndata;
	(*px)[j]+=tmp2*tmpent1.n/ndata;
	}
   }
}

void Density::Fx(Stat* Stats, int kx, Vector *x, double *fx)
{
int     i, j, ndata;
double  tmp;
Leaf    *tmpleaf1;
Entry   tmpent1;
tmpent1.Init(Stats->Dimension);

ndata=Stats->NewRoot->N();

for (i=0; i<kx; i++) 
	fx[i]=0.0;

// calculate the values by scanning all CF-kernels
i=0;
tmpleaf1=Stats->NewLeafHead;
while (Stats->NextEntryFromLeafHead(i,tmpent1,&tmpleaf1)!=FALSE) {
   for (j=0; j<kx; j++) {
	switch (Stats->CFDistr) {
	case 0: tmp=tmpent1.Unif_Kernel_Density_Effect(x[j],Stats->H);
		break;
	case 1: tmp=tmpent1.Norm_Kernel_Density_Effect(x[j],Stats->H);
		break;
	}
	fx[j]+=tmp*tmpent1.n/ndata;
	}
   }
}

void Density::Px(Stat* Stats, int kx, Vector *x, double *px)
{
int     i, j, ndata;
double  tmp;
Leaf    *tmpleaf1;
Entry   tmpent1;
tmpent1.Init(Stats->Dimension);

ndata=Stats->NewRoot->N();

for (i=0; i<kx; i++) 
	px[i]=0.0;

// calculate the values by scanning all CF-kernels
i=0;
tmpleaf1=Stats->NewLeafHead;
while (Stats->NextEntryFromLeafHead(i,tmpent1,&tmpleaf1)!=FALSE) {
   for (j=0; j<kx; j++) {
	switch (Stats->CFDistr) {
	case 0: tmp=tmpent1.Unif_Kernel_Prob_Effect(x[j],Stats->H);
		break;
	case 1: tmp=tmpent1.Norm_Kernel_Prob_Effect(x[j],Stats->H);
		break;
	}
	px[j]+=tmp*tmpent1.n/ndata;
	}
   }
}

void Density::FPx(Stat* Stats, int kx, Vector *x, double *fx, double *px)
{
int     i, j, ndata;
double  tmp1,tmp2;
Leaf    *tmpleaf1;
Entry   tmpent1;
tmpent1.Init(Stats->Dimension);

ndata=Stats->NewRoot->N();

for (i=0; i<kx; i++) {
	fx[i]=0.0;
	px[i]=0.0;
	}

// calculate the values by scanning all CF-kernels
i=0;
tmpleaf1=Stats->NewLeafHead;
while (Stats->NextEntryFromLeafHead(i,tmpent1,&tmpleaf1)!=FALSE) {
   for (j=0; j<kx; j++) {
	switch (Stats->CFDistr) {
	case 0: tmp1=tmpent1.Unif_Kernel_Density_Effect(x[j],Stats->H);
	        tmp2=tmpent1.Unif_Kernel_Prob_Effect(x[j],Stats->H);
		break;
	case 1: tmp1=tmpent1.Norm_Kernel_Density_Effect(x[j],Stats->H);
	        tmp2=tmpent1.Norm_Kernel_Prob_Effect(x[j],Stats->H);
		break;
	}
	fx[j]+=tmp1*tmpent1.n/ndata;
	px[j]+=tmp2*tmpent1.n/ndata;
	}
   }
}

void Density:: CF_Kernel_Smooth(Stat *Stats)
{
CF_Kernel_Start(Stats,1);
FPk(Stats,Stats->Bars,&X,&FX,&PX);
} 

void BirchDensity(Stat **Stats)
{
Density  *Dens;
char 	 tmpname[MAX_NAME];
ofstream tmpfile;

for (int i=0; i<Paras->ntrees; i++) {
	sprintf(tmpname,"%s-den-prob",Stats[i]->name);
	tmpfile.open(tmpname);
	Dens=new Density();
	Dens->CF_Kernel_Smooth(Stats[i]);
	Dens->Print_Den_Prob(tmpfile,Stats[i]);
	delete Dens;
	tmpfile.close();
	}
}


