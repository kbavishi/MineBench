/****************************************************************
File Name:   cfentry.C
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
#include "vector.h"
#include "rectangle.h"
#include "cfentry.h"

Entry::Entry() {}

void Entry::Init(short d) { 

n=0; sx.Init(d); sxx=0; 

#ifdef RECTANGLE
rect.Init(d);
#endif RECTANGLE
}

void Entry::Reset() {

n=0; sx.Reset(); sxx=0;

#ifdef RECTANGLE
rect.Reset();
#endif RECTANGLE
}
	
Entry::~Entry() {}

// correct copy constructor
Entry::Entry(const Entry& r) {
if (this!=&r) {
	n = r.n;
	sx = r.sx;
	sxx = r.sxx;

#ifdef RECTANGLE
	rect = r.rect;
#endif RECTANGLE
}
}
	
// correct assignment operator
void Entry::operator=(const Entry& r) {
if (this!=&r) {
	n = r.n;
	sx = r.sx;
	sxx = r.sxx;

#ifdef RECTANGLE
	rect = r.rect;
#endif RECTANGLE
	}
}	

void Entry::operator=(const int val) {
	n = val;
	sx = val;
	sxx = val;

#ifdef RECTANGLE
	rect = val;
#endif RECTANGLE
	}

void Entry::operator=(const Vector &v) {
	n = 1;
	sx = v;
	sxx = sx && sx;

#ifdef RECTANGLE
	rect = v;
#endif RECTANGLE
	}

short Entry::Dim() const {return sx.Dim();}

int Entry::N() const {return n;}

void Entry::SX(Vector& tmpsx) const {tmpsx = sx;}

double Entry::SXX() const {return sxx;}

void Entry::X0(Vector &tmpx0) const {tmpx0.Div(sx,n);}

#ifdef RECTANGLE
void Entry::Rect(Rectangle& tmprect) const {tmprect = rect;}
#endif RECTANGLE

// Assume:
// K(t) is standard normal distribution.
// gi(x) is normal distribution with its own xi and ri.
// wi(x) expanded only to o(h^2) with Taylor Expansions.
// work for $d$ dimensions.

double Entry::Norm_Kernel_Density_Effect(const Vector &x, double h) const {
double ri;
double tmp0=0;
double tmp1=0;

// w(x) = Norm(x0, h^2+r^2)

ri = sqrt(this->Radius()/sx.dim+h*h);

for (short i=0; i<sx.dim; i++) {
    tmp0 = (x.value[i]-sx.value[i]/n)/ri;
    tmp1 -= tmp0*tmp0/2.0;
    }
return exp(tmp1)/pow(sqrt(2*PI)*ri,sx.dim);
}

// work for 1 dimension, need to generalize
double Entry::Norm_Kernel_Prob_Effect(const Vector& a, double h) const
{
double r2=Radius();
return 0.5*(1.0+erf((a.value[0]-sx.value[0]/n)/(sqrt(2.0*(r2+h*h)))));
}

// Works for 1-dim presently, need to generalize to d-dim.
double Norm_Kernel_Rf_Effect(const Entry& ent1, const Entry& ent2, double h)
{
double ri,rj;
double tmp0,tmp1,tmp2;

ri = ent1.Radius();
rj = ent2.Radius();
tmp0 = 2.0*h*h+ri+rj;
tmp1 = ent1.sx.value[0]/ent1.n-ent2.sx.value[0]/ent2.n;
tmp2 = tmp1*tmp1;
tmp1 = tmp2/tmp0;
return 1.0/(sqrt(2.0*PI*pow(tmp0,5.0)))*exp(-tmp1/2.0)*((tmp1-3.0)*(tmp1-3.0)-6);
}

// Assume:
// K(t) is standard normal distribution.
// gi(x) is uniform distribution with its own xi and ri.
// wi(x) is actually integrated. 
// work for $d$ dimensions.

double Entry::Unif_Kernel_Density_Effect(const Vector &x, double h) const {
double ri;
double tmp0;
double tmp1;
double tmp=1.0;

ri = this->Radius()/sx.dim;

if (ri<=0) return this->Norm_Kernel_Density_Effect(x,h);

else {  ri = sqrt(ri);
	tmp0 = sqrt(3.0)*ri;
	tmp1 = sqrt(2.0)*h;
	for (short i=0; i<sx.dim; i++) {
		tmp *=(erf((sx.value[i]/n+tmp0-x.value[i])/tmp1)
	      	      -erf((sx.value[i]/n-tmp0-x.value[i])/tmp1))
	              /(4.0*tmp0);
		}
	return tmp;
 	} 
}

// work for 1 dimension, need to generalize
double Entry::Unif_Kernel_Prob_Effect(const Vector &a, double h) const
{
double tmp0, tmp1, tmp2, tmp3, tmp4;
double r2=Radius();
if (r2<=0) 
  return 0.5*(1.0+erf((a.value[0]-sx.value[0]/n)/(sqrt(2.0)*h)));
else {
  tmp0=sqrt(3.0*r2);
  tmp1=sqrt(2.0)*h;
  tmp2=a.value[0]-sx.value[0]/n;
  tmp3=tmp2-tmp0;
  tmp4=tmp2+tmp0;
  return 0.5+1.0/(4.0*tmp0)*(-tmp3*erf(tmp3/tmp1)+tmp4*erf(tmp4/tmp1)
	                     +sqrt(2.0/PI)*h*(-exp(-tmp3/tmp1*tmp3/tmp1)
					      +exp(-tmp4/tmp1*tmp4/tmp1)));
  }
}

// Works for 1-dim presently, need to generalize to d-dim.
double Unif_Kernel_Rf_Effect(const Entry& ent1, const Entry& ent2, double h)
{
double ri,rj;
double xi,xj;
double ai,bi,aj,bj,tmp0,tmp1,tmp2;

ri=sqrt(ent1.Radius());
rj=sqrt(ent2.Radius());
xi=ent1.sx.value[0]/ent1.n;
xj=ent2.sx.value[0]/ent2.n;

tmp0 = sqrt(3.0)*ri;
ai=xi-tmp0;
bi=xi+tmp0;
tmp1 = sqrt(3.0)*rj;
aj=xj-tmp1;
bj=xj+tmp1;

if (ri<=0 && rj<=0) {
	tmp0=2*h*h;
	tmp1=(xi-xj)*(xi-xj);
	return 1.0/sqrt(2.0*PI*pow(tmp0,5.0))*exp(-tmp1/(2.0*tmp0))*((tmp1/tmp0-3.0)*(tmp1/tmp0-3.0)-6.0);
	}

if (ri>0 && rj>0) {
	tmp2=0.0;
	tmp0=h*h;
	tmp1=(bi-bj)*(bi-bj);
	tmp2+=exp(-tmp1/(4.0*tmp0))*(tmp0-tmp1/2.0);
	tmp1=(ai-bj)*(ai-bj);
	tmp2-=exp(-tmp1/(4.0*tmp0))*(tmp0-tmp1/2.0);
	tmp1=(bi-aj)*(bi-aj);
	tmp2-=exp(-tmp1/(4.0*tmp0))*(tmp0-tmp1/2.0);
	tmp1=(ai-aj)*(ai-aj);
	tmp2+=exp(-tmp1/(4.0*tmp0))*(tmp0-tmp1/2.0);
	return tmp2/(48*sqrt(PI)*ri*rj*h*h*h*h*h);
	}

if (ri<=0 && rj>0) {
	tmp2=0.0;
	tmp0=h*h;
	tmp1=(xi-bj)/2.0;
	tmp2+=exp(-1.0/tmp0*tmp1*tmp1)*(1.5*tmp1-tmp1*tmp1*tmp1/tmp0);
	tmp1=(xi-aj)/2.0;
	tmp2-=exp(-1.0/tmp0*tmp1*tmp1)*(1.5*tmp1-tmp1*tmp1*tmp1/tmp0);
	return tmp2/(-4.0*sqrt(3.0*PI)*rj*h*h*h*h*h);
	}

if (ri>0 && rj<=0) {
	tmp2=0.0;
	tmp0=h*h;
	tmp1=(xj-bi)/2.0;
	tmp2+=exp(-1.0/tmp0*tmp1*tmp1)*(1.5*tmp1-tmp1*tmp1*tmp1/tmp0);
	tmp1=(xj-ai)/2.0;
	tmp2-=exp(-1.0/tmp0*tmp1*tmp1)*(1.5*tmp1-tmp1*tmp1*tmp1/tmp0);
	return tmp2/(-4.0*sqrt(3.0*PI)*ri*h*h*h*h*h);
	}
}

double Entry::Diameter() const {
	if (n<=1) return 0;
	else // return 2*(n*sxx-(sx&&sx))/(n*(n-1));
	     // for performance reason
	     {double tmp1,tmp0=0;
	      for (short i=0; i<sx.dim; i++) 
		tmp0 += sx.value[i]/n*sx.value[i]/(n-1);
	      tmp1=2*(sxx/(n-1.0)-tmp0);
	      if (tmp1<=0.0) return 0.0;
	      else return tmp1;
	      }
}

double Entry::Radius() const {
	if (n<=1) return 0;
	else // return(sxx/n-((sx/n)&&(sx/n)));
	     // for performance reason
		{double tmp0, tmp1=0;
		 for (short i=0; i<sx.dim; i++) {
			tmp0 = sx.value[i]/n;
			tmp1 += tmp0*tmp0;
			}
		 tmp0=sxx/n-tmp1;
		 if (tmp0<=0.0) return 0.0;
		 else return tmp0;
		}
}

double Entry::Fitness(short ftype) const {
	switch (ftype) {
	case AVG_DIAMETER: return Diameter();
	case AVG_RADIUS:   return Radius();
	default: print_error("Entry::Fitness", "Invalid fitness type");
	}
}

void Entry::operator+=(const Entry &ent) {
	n += ent.n;
	sx += ent.sx;
	sxx += ent.sxx;

#ifdef RECTANGLE
	rect += ent.rect;
#endif RECTANGLE
	}

void Entry::operator-=(const Entry &ent) {
	n -= ent.n;
	sx -= ent.sx;
	sxx -= ent.sxx;

#ifdef RECTANGLE
	rect = rect; // Be careful that rect can not be restored
#endif RECTANGLE
	}

// centroid euclidian distance D0	
double Entry::operator||(const Entry& v2) const {
// return((sx/n) || (v2.sx/v2.n));
// for space allocation, performance and overflow reason
double tmp, d = 0;
for (short i=0; i<sx.dim; i++) {
	tmp = sx.value[i]/n - v2.sx.value[i]/v2.n;
	d += tmp*tmp;
	}
if (d>=0) return d; else return 0.0;
}

// centroid manhatan distance  D1
double Entry::operator^(const Entry &v2) const {
// return((sx/n) ^ (v2.sx/v2.n));
// for space allocation, performance and overflow reason
double tmp, d = 0;
for (short i=0; i<sx.dim; i++) {
	tmp = fabs(sx.value[i]/n - v2.sx.value[i]/v2.n);
	d += tmp;
	}
if (d>=0) return d; else return 0.0;
}

// inter-cluster distance D2
double Entry::operator|(const Entry& v2) const {
double d=(v2.n*sxx+n*v2.sxx-2*(sx&&v2.sx))/(n*v2.n);
if (d>=0) return d; else return 0.0;
}

// intra-cluster distance D3 (bad)
double Entry::operator&(const Entry& v2) const {
// return(2*((n+v2.n)*(sxx+v2.sxx)-
//           ((sx+v2.sx)&&(sx+v2.sx)))/((n+v2.n)*(n+v2.n-1)));
// for space allocation, performance and overflow reason
int tmpn = n+v2.n;
double tmp1, tmp2=0;
for (short i=0; i<sx.dim; i++) {
	tmp1 = sx.value[i]+v2.sx.value[i];
	tmp2 += tmp1/tmpn*tmp1/(tmpn-1);
	}
double d=2*((sxx+v2.sxx)/(tmpn-1)-tmp2);
if (d>=0) return d; else return 0.0;
}

// variance increase distance D4 
double Entry::operator&&(const Entry& v2) const {
// return(n*((sx/n)&&(sx/n))+
//        v2.n*((v2.sx/v2.n)&&(v2.sx/v2.n))-
//        (n+v2.n)*(((sx+v2.sx)/(n+v2.n))&&((sx+v2.sx)/(n+v2.n))));
// for space allocation, performance and overflow reason
double tmp1, tmp2, tmp3;
double dot1, dot2, dot3;
dot1 = dot2 = dot3 =0;
for (short i=0; i<sx.dim; i++) {
	tmp1 = sx.value[i]/n;
	dot1 += tmp1*tmp1;
	tmp2 = v2.sx.value[i]/v2.n;
	dot2 += tmp2*tmp2;
	tmp3 = (sx.value[i]+v2.sx.value[i])/(n+v2.n);
	dot3 += tmp3*tmp3;
	}
double d=(n*dot1+v2.n*dot2-(n+v2.n)*dot3);
if (d>=0) return d; else return 0.0;
}

void Entry::Add(const Entry& e1, const Entry& e2) {
	n = e1.n+e2.n;
	sx.Add(e1.sx, e2.sx);
	sxx = e1.sxx+e2.sxx;

#ifdef RECTANGLE
	rect.Add(e1.rect, e2.rect); // rect changed
#endif RECTANGLE
	}

void Entry::Sub(const Entry& e1, const Entry& e2) {
	n = e1.n-e2.n;
	sx.Sub(e1.sx, e2.sx);
	sxx = e1.sxx-e2.sxx;
#ifdef RECTANGLE
	rect = e1.rect; // Be careful that rect can not be restored.
#endif RECTANGLE
	}

void Entry::Transform(const Vector &W, const Vector &M) {
if (n<=1) {
	sx.Transform(W,M);

#ifdef RECTANGLE
	rect.Transform(W,M);
#endif RECTANGLE

	sxx = sx && sx;
	}
}

// correct for dimension=1,2,3, needs to verify for dimension>3
// with d-dimensional hyper-sphere volume formula

// connect nonleaf entries
short connected(const Entry& ent1, const Entry& ent2)
{
double sqrR1, sqrR2, d;
sqrR1=ent1.Radius()*(ent1.sx.dim+2.0)/ent1.sx.dim;
sqrR2=ent2.Radius()*(ent2.sx.dim+2.0)/ent2.sx.dim;
d=ent1||ent2;
if (sqrt(d)<=sqrt(sqrR1)+sqrt(sqrR2)) return TRUE;
else return FALSE;
}

// connect leaf entries
short connected(const Entry& ent1, const Entry& ent2, 
		short ftype, double Ft,double density)
{
double d=ent1||ent2; 
switch (ftype) {
  case AVG_DIAMETER: if (sqrt(d)<2*sqrt(Ft/2.0*(ent1.sx.dim+2.0)/ent1.sx.dim) &&
			 ent1.n>density && ent2.n>density) 
	             return TRUE; 
		     else return FALSE;
  case AVG_RADIUS:   if (sqrt(d)<2*sqrt(Ft*(ent1.sx.dim+2.0)/ent1.sx.dim) && 
			 ent1.n>density && ent2.n>density) 
		     return TRUE; 
		     else return FALSE; 
  default: print_error("connected", "Invalid fitness type");
  }
}

/* Intended for generating data files for Devise */

void Entry::Visualize_Circle(ostream &fo) const {
Vector tmpv;
tmpv.Init(sx.dim);
if (sx.dim>2)
	print_error("Entry::Visualize",
		    "can't visualize higher than 2 dimensions");
tmpv.Div(sx,n);
if (n>1) {
fo << 3 << " "  	// shape: filled polygon
   << n%10+3 << " "     // color: variable
   << n << " "					// count
   << tmpv << " " 				// center
   << 2*sqrt(Radius()) << " "		//width
   << 2*sqrt(Radius()) << endl;       //height
   }
else {
fo << 3 << " "
   << n%10+3 << " "
   << n << " "
   << tmpv << " "
   << 0 << " "
   << 0 << endl;
   }
}

void Entry::Visualize_Rectangle(ostream &fo) const {
Vector tmpv;
tmpv.Init(sx.dim);
if (sx.dim>2)
	print_error("Entry::Visualize",
			    "can't visualize higher than 2 dimensions");
tmpv.Div(sx,n);
fo << 0 << " "   // shape: filled rectangle
   << 2 << " "   // color: red
   << n << " "
#ifdef RECTANGLE
   << rect << " "
#endif RECTANGLE
   << endl;  	// center, width, height
}

void Entry::Visualize_Circle(ofstream &fo) const {
Vector tmpv;
tmpv.Init(sx.dim);
if (sx.dim>2)
	print_error("Entry::Visualize",
		    "can't visualize higher than 2 dimensions");
tmpv.Div(sx,n);
if (n>1) {
fo << 3 << " "
   << n%10+3 << " "
   << n << " "
   << tmpv << " " 
   << 2*sqrt(Radius()) << " "
   << 2*sqrt(Radius()) << endl;
   }
else {
fo << 3 << " "
   << n%10+3 << " "
   << n << " "
   << tmpv << " "
   << 0 << " "
   << 0 << endl;
   }
}

void Entry::Visualize_Rectangle(ofstream &fo) const {
Vector tmpv;
tmpv.Init(sx.dim);
if (sx.dim>2)
	print_error("Entry::Visualize",
			    "can't visualize higher than 2 dimensions");
tmpv.Div(sx,n);
fo << 0 << " "
   << 2 << " "
   << n << " "
#ifdef RECTANGLE
   << rect << " "
#endif RECTANGLE
   << endl;
}

istream &operator>=(istream &fi, Entry &ent) {
	ent.n=1;
	fi >> ent.sx;
	ent.sxx = ent.sx && ent.sx;

#ifdef RECTANGLE
	ent.rect=ent.sx;
#endif RECTANGLE
	return fi;
	}

ifstream &operator>=(ifstream &fi, Entry &ent) {
	ent.n=1;
	fi >> ent.sx;
	ent.sxx = ent.sx && ent.sx;

#ifdef RECTANGLE
	ent.rect=ent.sx;
#endif RECTANGLE
	return fi;
	}

istream &operator>>(istream &fi, Entry &ent) {
	fi >> ent.n;
	fi >> ent.sx;
	fi >> ent.sxx;

#ifdef RECTANGLE
	fi >> ent.rect;
#endif RECTANGLE
	return fi;
	}

ifstream &operator>>(ifstream &fi, Entry &ent) {
	fi >> ent.n;
	fi >> ent.sx;
	fi >> ent.sxx;

#ifdef RECTANGLE
	fi >> ent.rect;
#endif RECTANGLE
	return fi;
	}

ostream &operator<<(ostream &fo, const Entry &r) {
	fo << r.n <<'\t';
	//fo << r.sx <<'\t'; 
	for (int i=0; i < r.sx.dim; i++)
		fo << r.sx.value[i]/r.n << '\t';
	fo << r.sxx/r.n <<'\t';

	fo << sqrt(r.Radius()) << '\t';
	fo << sqrt(r.Diameter()) << '\t';

#ifdef RECTANGLE
	fo << r.rect << '\t';
#endif RECTANGLE
	return fo;
}

ofstream &operator<<(ofstream &fo, const Entry &r) {
	fo << r.n <<'\t';
	//fo << r.sx <<'\t';
	for (int i=0; i < r.sx.dim; i++)
		fo << r.sx.value[i]/r.n << '\t';
	fo << sqrt(r.Radius()) << '\t';
	//fo << r.sxx <<'\t';

	fo << sqrt(r.Diameter()) << '\t';

#ifdef RECTANGLE
	fo << r.rect << '\t';
#endif RECTANGLE
	return fo;
}

