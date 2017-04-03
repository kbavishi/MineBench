/****************************************************************
File Name:   vector.C
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
//#include "util.h"
#include "vector.h"
	
Vector::Vector() {value=NULL;}

void Vector::Init(short d) {
	dim = d;
	value = new double[dim];
	memset(value, 0, dim*sizeof(double));
	}

void Vector::Reset() {
	memset(value, 0, dim*sizeof(double));
	}

Vector::Vector(const Vector& r) {
	dim = r.dim;
	memcpy(value,r.value,dim*sizeof(double));
	}

Vector::~Vector() {if (value!=NULL) delete [] value;}

double Vector::Value(short i) const {return value[i];}

void Vector::operator=(const Vector& r) {
	if (this != &r) {
		dim = r.dim;
		memcpy(value,r.value,dim*sizeof(double));
		}
	}
	
void Vector::operator=(const int val) {
	for (short i=0; i<dim; i++)
		value[i] = val;
}

void Vector::operator=(const float val) {
	for (short i=0; i<dim; i++) 
		value[i]=val;
	}

void Vector::operator=(const double val) {
	for (short i=0; i<dim; i++)
		value[i]=val;
	}


// euclidian distance : sum of squares on each dimension
double Vector::operator||(const Vector& v2) const {
	double tmp = 0;
	for (short i=0; i<dim; i++) 
		tmp += (value[i]-v2.value[i])*(value[i]-v2.value[i]);
	return tmp;
	}
	
// manhatan distance : sum of absolute values on each dimension
double Vector::operator^(const Vector& v2) const {
	double tmp = 0;
	for (short i=0; i<dim; i++) 
		tmp += fabs(value[i]-v2.value[i]);
	return tmp;
	}

void Vector::operator+=(const Vector &v2) {
	for (short i=0; i<dim; i++) 
		value[i] += v2.value[i];
	}

void Vector::operator-=(const Vector &v2) {
	for (short i=0; i<dim; i++) 
		value[i] -= v2.value[i];
	}

void Vector::operator*=(double cnt) {
	for (short i=0; i<dim; i++) 
		value[i] *= cnt;
	}

void Vector::operator/=(double cnt) {
	for (short i=0; i<dim; i++) 
		value[i] /= cnt;
	}

// dot product
double Vector::operator&&(const Vector& v2) const {
	double tmp = 0;
	for (short i=0; i<dim; i++) 
		tmp += value[i]*v2.value[i];
	return tmp;
	}

// time by N
void Vector::Mul(const Vector& v1, double N) {
	for (short i=0;i<dim;i++)
		value[i]=v1.value[i]*N;
	}
	
// divide by N
void Vector::Div(const Vector& v1, double N) {
	for (short i=0;i<dim;i++)
		value[i]=v1.value[i]/N;
	}

// plus
void Vector::Add(const Vector& v1, const Vector& v2) {
	for (short i=0; i<dim; i++)
		value[i]=v1.value[i]+v2.value[i];
	}

// minus
void Vector::Sub(const Vector& v1, const Vector& v2) {
	for (short i=0; i<dim; i++)
		value[i]=v1.value[i]-v2.value[i];
	}

void Vector::AddSqr(const Vector& v) {
	for (short i=0;i<dim;i++)
		value[i]+=v.value[i]*v.value[i];
	}

// weighting and moving
void Vector::Transform(const Vector &W,const Vector &M) {
	for (short i=0; i<dim; i++)
        	value[i]=(value[i]-M.value[i])*W.value[i];
	}

// reverse weighting and reverse moving
void Vector::Reverse_Transform(const Vector &W, const Vector &M) {
	for (short i=0; i<dim; i++)
		value[i]=value[i]/W.value[i]+M.value[i];
	}

// input a vector
istream &operator>>(istream &fi, Vector &r) {
	for (short i=0;i<r.dim;i++) 
		fi >> r.value[i];
	return fi;
	}

ifstream &operator>>(ifstream &fi, Vector &r) {
	for (short i=0;i<r.dim;i++) 
		fi >> r.value[i];
	return fi;
	}
	
// output a vector
ostream &operator<<(ostream &fo, const Vector &r) {
	for (short i=0; i<r.dim; i++)
		fo << r.value[i] << ' '; 
	return fo;
	}

ofstream &operator<<(ofstream &fo, const Vector &r) {
	for (short i=0; i<r.dim; i++)
		fo << r.value[i] << ' '; 
	return fo;
	}


