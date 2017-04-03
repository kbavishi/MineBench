/****************************************************************
File Name:   rectangle.h
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef RECTANGLE_H
#define RECTANGLE_H

// spatial object class: n-dimensional rectengles 
// are represented as two points: 
// For example, (xlow, ylow, zlow) (xhigh, yhigh, zhigh) (3 dimension). 

class Rectangle {
public:
	short  dim;
	double *low;
	double *high;  

	Rectangle();
	void Init(short d);
	void Reset();
	Rectangle(const Rectangle& rect);
	~Rectangle();

	void operator=(const Rectangle& rect);
	void operator=(const int val);
	void operator=(const float val);
	void operator=(const double val);
	void operator=(const Vector &v);
	
	short Dim() const {return dim;}

	const double *LowBound() const;
	const double *HighBound() const;
	double LowBound(short n) const;
	double HighBound(short n) const;
	double Middle(short n) const;
	double Length(short n) const;

// area of a Rectangle :
//	>0 : valid rectangle
//	=0 : a point
//	-1 : null rectangle 
//
	double Area() const;

// margin of a Rectangle

	double Margin() const;
	
	void Assign(const Vector &lv, const Vector &hv);
	void Transform(const Vector &W, const Vector &M);

// some binary operations:
//	* intersection  ->  rectangle
//	+ addition      ->  rectangle
//	+= enlarge by adding the new box
//	*= delarge by intersection
//	== exact match  ->  boolean
//	> containment   ->  boolean
//	^ overlap	->  boolean
//	|| distance (sum of squares) between centers of two rectangles 

	// Rectangle operator*(const Rectangle& other) const;
	void Mul(const Rectangle& r1, const Rectangle& r2);
	// Rectangle operator+(const Rectangle& other) const;
	void Add(const Rectangle& r1, const Rectangle& r2);
	void operator+=(const Vector& other);
	void operator+=(const Rectangle& other);
	void operator*=(const Rectangle& other);
	short operator==(const Rectangle& other) const;
	short operator>(const Rectangle& other) const;
	short operator^(const Rectangle& other) const;
	double operator||(const Rectangle& other) const;

	friend istream &operator>>(istream &fi, Rectangle &rect);
	friend ifstream &operator>>(ifstream &fi, Rectangle &rect);
	friend ostream &operator<<(ostream &fo, const Rectangle &rect);
	friend ofstream &operator<<(ofstream &fo, const Rectangle &rect);
	
	friend double Point_Kernel_Effect(Vector &x, Vector &centroid, double H);
};

istream &operator>>(istream &fi, Rectangle &rect);
ifstream &operator>>(ifstream &fi, Rectangle &rect);
ostream &operator<<(ostream &fo, const Rectangle &rect);
ofstream &operator<<(ofstream &fo, const Rectangle &rect);

#endif RECTANGLE_H

