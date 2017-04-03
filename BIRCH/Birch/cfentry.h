/****************************************************************
File Name:   cfentry.h
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef CFENTRY_H
#define CFENTRY_H

class Entry {
public: 
	int n;
	Vector sx;
	double sxx;

#ifdef RECTANGLE
	Rectangle rect;
#endif

	Entry();
	void Init(short d);
	void Reset();
        Entry (const Entry& ent);
	~Entry();	

	void operator=(const Entry& ent);
	void operator=(const int val);
	void operator=(const Vector& v);

	short Dim() const;
	int N() const;
	void SX(Vector &tmpsx) const;
	double SXX() const;

	// can be calculated from n,sx,sxx

	void X0(Vector &tmpx0) const;
	double Diameter() const;
	double Radius() const;
	double Fitness(short ftype) const;

	double Norm_Kernel_Density_Effect(const Vector &x, double h) const;
	double Norm_Kernel_Prob_Effect(const Vector &a, double h) const; 

	double Unif_Kernel_Density_Effect(const Vector &x, double h) const;
	double Unif_Kernel_Prob_Effect(const Vector &a, double h) const; 


#ifdef RECTANGLE
	// Rectangle Rect() const;
	void Rect(Rectangle &tmprect) const;
#endif

	void operator+=(const Entry &ent);
	void operator-=(const Entry &ent);
	// Entry operator+(const Entry& ent) const;
	void Add(const Entry& e1, const Entry& e2);
	// Entry operator-(const Entry& ent) const;
	void Sub(const Entry& e1, const Entry& e2);

	void Transform(const Vector &W, const Vector &M);

	void Visualize_Circle(ostream &fo) const;
	void Visualize_Circle(ofstream &fo) const;
	void Visualize_Rectangle(ostream &fo) const;
	void Visualize_Rectangle(ofstream &fo) const;

	// D0 euclidian distance of centroids
	double operator||(const Entry& ent) const;
	// D1 manhatan distance of centroids
	double operator^(const Entry& ent) const;
	// D2 inter-cluster distance before merge
	double operator|(const Entry& ent) const; 
	// D3 intra-cluster distance after merge
	double operator&(const Entry& ent) const;
	// variance increase distance D4
	double operator&&(const Entry& v2) const;

// input entry 
friend istream &operator>>(istream &fi, Entry &ent); 
friend ifstream &operator>>(ifstream &fi, Entry &ent);

// input data point as entry:
friend istream &operator>=(istream &fi, Entry &ent);
friend ifstream &operator>=(ifstream &fi, Entry &ent);

// output entry
friend ostream &operator<<(ostream &fo, const Entry &ent);
friend ofstream &operator<<(ofstream &fo, const Entry &ent);

// connectivity tests
friend short connected(const Entry& ent1, const Entry& ent2);
friend short connected(const Entry& ent1, const Entry& ent2, short ftype, double Ft, double density);

// Rf'' effects 
friend double Norm_Kernel_Rf_Effect(const Entry& ent1, const Entry& ent2,double h);
friend double Unif_Kernel_Rf_Effect(const Entry& ent1, const Entry& ent2,double h);
};

istream &operator>>(istream &fi, Entry &ent); 
ifstream &operator>>(ifstream &fi, Entry &ent);

istream &operator>=(istream &fi, Entry &ent);
ifstream &operator>=(ifstream &fi, Entry &ent);

ostream &operator<<(ostream &fo, const Entry &ent);
ofstream &operator<<(ofstream &fo, const Entry &ent);

short connected(const Entry& ent1, const Entry& ent2);
short connected(const Entry& ent1, const Entry& ent2, short ftype, double Ft);

double Norm_Kernel_Rf_Effect(const Entry& ent1, const Entry& ent2,double h);
double Unif_Kernel_Rf_Effect(const Entry& ent1, const Entry& ent2,double h);

#endif CFENTRY_H

