/****************************************************************
File Name:   vector.h
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

#ifndef VECTOR_H
#define VECTOR_H

class Vector {

public:
  short  dim;
  double *value;
  
  double Value(short i) const;
  
  Vector();
  void Init(short d);
  void Reset();
  
  Vector(const Vector& r);
  ~Vector();
  void operator=(const Vector& r);
  
  short Dim() const {return dim;}
  
  //VENKY
  void newArr(int dimensionality, double *arr)
    {
      dim = dimensionality;
      value = arr;
    }
  void Nullify()
    {
      dim = 0;
      value = NULL;
    }
  void SetVal(int i, double val)
    {
      assert(i < dim);
      value[i] = val;
    }
  void SetDim(int dimn)
    {
      dim = dimn;
    }
  // euclidian distance between two vectors
  double operator||(const Vector& v2) const;
  // manhatan distance between two vectors
  double operator^(const Vector& v2) const;
  
  void operator+=(const Vector& v2);
  void operator-=(const Vector& v2);
  void operator*=(double cnt);
  void operator/=(double cnt);
  
  // dot product
  double operator&&(const Vector& v2) const;
  // divide by N
  // Vector operator/(double N) const;
  void Div(const Vector& v1, double N);
  // time by N
  // Vector operator*(double N) const;
  void Mul(const Vector& v1, double N);
  // plus
  // Vector operator+(const Vector& v2) const;
  void Add(const Vector& v1, const Vector& v2);
  // minus
  // Vector operator-(const Vector& v2) const;
  void Sub(const Vector& v1, const Vector& v2);
  
  void AddSqr(const Vector& v);
  
  
  void Transform(const Vector &W, const Vector &M);
  void Reverse_Transform(const Vector &W, const Vector &M);
  
  void operator=(const int val);
  void operator=(const float val);
  void operator=(const double val);
  
  friend class Rectangle;
  friend class Node;
  friend class Entry;
  
  friend void perturb(Vector &oldcodeword,Vector &newcodeword,int scale);
  friend void splitcodewords(Vector *codes,int oldsize,int newsize,int scale);
  // I/O and file I/O
  friend istream &operator>>(istream &fi,Vector &v);
  friend ifstream &operator>>(ifstream &fi,Vector &v);
  friend ostream &operator<<(ostream &fo,const Vector &v);
  friend ofstream &operator<<(ofstream &fo,const Vector &v);
  
  friend double Point_Kernel_Effect(Vector &x, Vector &centroid, double H);
};

istream &operator>>(istream &fi,Vector &v);
ifstream &operator>>(ifstream &fi,Vector &v);
ostream &operator<<(ostream &fo,const Vector &v);
ofstream &operator<<(ofstream &fo,const Vector &v);

#endif VECTOR_H

