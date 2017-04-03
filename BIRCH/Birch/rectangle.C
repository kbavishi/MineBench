/****************************************************************
File Name:   rectangle.C 
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

Rectangle::Rectangle() {}

void Rectangle::Init(short d)
{
dim = d;
low = new double[dim];
high = new double[dim];
for (short i=0; i<dim; i++) {
	low[i]=0.0;
	high[i]=-1.0;
	}
}

void Rectangle::Reset() 
{
for (short i=0; i<dim; i++) {
	low[i]=0;
	high[i]=-1;
	}
}

Rectangle::Rectangle(const Rectangle& rect)
{
	dim = rect.dim;
	memcpy(low,rect.low,dim*sizeof(double));
	memcpy(high,rect.high,dim*sizeof(double));
}

Rectangle::~Rectangle() 
{
if (low!=NULL) delete [] low; 
if (high!=NULL) delete [] high;
}

void Rectangle::operator=(const Rectangle &rect) 
{
if (this!=&rect) {
		dim = rect.dim;
		memcpy(low,rect.low,dim*sizeof(double));
		memcpy(high,rect.high,dim*sizeof(double));
		}
}

void Rectangle::operator=(const int val)
{
for (short i=0; i<dim; i++) {
	low[i] = val; high[i] = val;
	}
}

void Rectangle::operator=(const float val)
{
for (short i=0; i<dim; i++) {
	low[i] = val; high[i] = val;
	}
}

void Rectangle::operator=(const double val)
{
for (short i=0; i<dim; i++) {
	low[i] = val; high[i] = val;
	}
}

void Rectangle::operator=(const Vector &v)
{
	memcpy(low,v.value,dim*sizeof(double));
	memcpy(high,v.value,dim*sizeof(double));
}
	
const double* Rectangle::LowBound() const { return low;}
const double* Rectangle::HighBound() const { return high;}
double Rectangle::LowBound(short n) const { return low[n];}
double Rectangle::HighBound(short n) const { return high[n];}
double Rectangle::Middle(short n) const { return (low[n]+high[n])/2; }
double Rectangle::Length(short n) const { return (high[n]-low[n]); }

double Rectangle::Area() const {
double area = 1;
for (short i=0; i<dim; i++) {
	if (high[i] < low[i]) return -1;
	area *= (high[i] - low[i]);
	}
return area;
}

double Rectangle::Margin() const {
double margin = 0;
for (short i=0; i<dim; i++) {
	if (high[i] < low[i]) return -1; 
	margin += (high[i] - low[i]); 
	}
return margin;
}

void Rectangle::Assign(const Vector &lv, const Vector &hv) {
	memcpy(low,lv.value,dim*sizeof(double));
	memcpy(high,hv.value,dim*sizeof(double));
	}

void Rectangle::Transform(const Vector &W, const Vector &M) {
for (short i=0; i<dim; i++) {
	low[i] = (low[i]-M.Value(i))*W.Value(i);
	high[i] = (high[i]-M.Value(i))*W.Value(i);
	}
}

// if two rectangles are not intersected, 
// then result will be a null rectangle.

void Rectangle::Mul(const Rectangle& r1, const Rectangle& r2) {
if (r1.low[0]>r1.high[0]) {*this = r2; return;}
if (r2.low[0]>r2.high[0]) {*this = r1; return;}
for (short i=0; i<dim; i++) {
	low[i] = MAX(r1.low[i], r2.low[i]);
	high[i] = MIN(r1.high[i], r2.high[i]); 
	}
}

void Rectangle::Add(const Rectangle& r1, const Rectangle &r2) {
if (r1.low[0]>r1.high[0]) {*this = r2; return;}
if (r2.low[0]>r2.high[0]) {*this = r1; return;}
for (short i=0; i<dim; i++) {
	low[i] = MIN(r1.low[i], r2.low[i]);
	high[i] = MAX(r1.high[i], r2.high[i]);
	}
}

void Rectangle::operator+=(const Vector& other)
{
if (low[0]>high[0]) {
	for (short i=0; i<dim; i++) {
		low[i]=other.value[i];
		high[i]=low[i];
		}
	}
else {  for (short i=0; i<dim; i++) {
		low[i]=MIN(low[i],other.value[i]);
		high[i]=MAX(high[i],other.value[i]);
		}
	}
}

void Rectangle::operator+=(const Rectangle& other)
{
if (low[0]>high[0]) {*this=other; return;}
if (other.low[0]>other.high[0]) return;
for (short i=0; i<dim; i++) {
	low[i] = MIN(low[i], other.low[i]);
	high[i] = MAX(high[i], other.high[i]);
	}
}

void Rectangle::operator*=(const Rectangle& other)
{
if (low[0]>high[0]) {*this=other; return;}
if (other.low[0]>other.high[0]) return;
for (short i=0; i<dim; i++) {
	low[i] = MAX(low[i], other.low[i]);
	high[i] = MIN(high[i], other.high[i]);
	}
}

//equal
short Rectangle::operator==(const Rectangle& other) const
{
short i;
for (i=0; i<dim; i++) {
	if (ABS(low[i]-other.low[i])>ERROR || 
	    ABS(high[i]-other.high[i])>ERROR)
	return FALSE;
	}
return TRUE;
}

//contain
short Rectangle::operator>(const Rectangle& other) const
{
short i;
for (i=0; i<dim; i++) 
	if (low[i] > other.low[i]) return FALSE;
for (i=0; i<dim; i++) 
	if (high[i] < other.high[i]) return FALSE;
return TRUE;
}

//overlap
short Rectangle::operator^(const Rectangle& other) const
{
	Rectangle tmp;
	tmp.Init(dim);
	tmp.Mul(*this,other);
	return (tmp.Area() >= 0);
}

double Rectangle::operator||(const Rectangle& other) const
{
double square = 0;
for (short i=0; i<dim; i++) {
	double diff = low[i] + high[i] - other.low[i] - other.high[i];
	square += (diff*diff/4);
	}
return square;
}

istream &operator>>(istream &fi, Rectangle &rect) {
	register short i;
	for (i=0;i<rect.dim;i++)  fi >> rect.low[i];
	for (i=0;i<rect.dim;i++)  fi >> rect.high[i];
	return fi;
	}

ifstream &operator>>(ifstream &fi, Rectangle &rect) {
	register short i;
	for (i=0;i<rect.dim;i++) fi >> rect.low[i];
	for (i=0;i<rect.dim;i++) fi >> rect.high[i];
	return fi;
	}

ostream &operator<<(ostream &fo, const Rectangle &rect) {
	register short i;
	for (i=0;i<rect.dim;i++) fo << rect.low[i] << ' ';
	for (i=0;i<rect.dim;i++) fo << rect.high[i] << ' ';
	return fo;
	}

ofstream &operator<<(ofstream &fo, const Rectangle &rect) {
	register short i;
	for (i=0;i<rect.dim;i++) fo << rect.low[i] << ' ';
	for (i=0;i<rect.dim;i++) fo << rect.high[i] << ' ';
	return fo;
	}









