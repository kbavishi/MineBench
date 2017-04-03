#ifndef BITMAP_H
#define BITMAP_H

#include <cmath>
#include <iostream>
#include <string.h>

using namespace std;

class Bitmap
{
 public:
  Bitmap();
  //  Bitmap(Bitmap rhs);
  Bitmap(int bits);

  int CountOnes() const;
  int CountOnesAfter(int after) const;

  void Clear();
 
  void SetBit(int bitNumber, bool value);
  bool GetBit(int bitNumber) const;
  int GetID() const;
  void SetID(int id);
  int LastOne() const;
  bool ContainedIn(Bitmap *rhs) const;

  static double Cosine(Bitmap *lhs, Bitmap *rhs);
  static double Correlation(Bitmap *lhs, Bitmap *rhs);

  //  bool operator<(const Bitmap &b);
  friend ostream & operator<<(ostream & out, Bitmap & bm);
  bool operator==(const Bitmap &rhs);
  Bitmap & operator=(const Bitmap &rhs);
  Bitmap & operator|=(const Bitmap &rhs);
  Bitmap & operator&=(const Bitmap &rhs);
  Bitmap & operator|(const Bitmap &rhs);
  Bitmap & operator&(const Bitmap &rhs);

 private:
  int id;
  int numberOfBits;
  int numberOfBytes;
  static int oneCount[256];
  char *column;
};


#endif
