#ifndef DATASET_H
#define DATASET_H

#include <bitset>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#include "Bitmap.h"

using namespace std;

class DataSet
{
 public:
  DataSet(string filename);

  int GetRemainingOnesInTransaction(int trans, int after) const;
  int GetTransactionCount() const;
  int GetItemCount() const;
  int GetActualIndex(int index) const;
  int * GetSupportSet(int searchFor, int *length);
  void AddItemToZeroCount(int *zeros, int item) const;
  void RemoveItemFromZeroCount(int *zeros, int item) const;
  bool ValueAt(int row, int col) const;
  Bitmap * GetBitmaps() const;
  Bitmap * GetItemBitmap(int index);
  friend ostream & operator<<(ostream & out, DataSet & d);

 private:
  //bitset<8> itemBits;
  vector<Bitmap> itemBits;
  vector<Bitmap> transBits;

  vector< vector <int> > theData;
  unsigned int **supportBitmaps;
  int numberOfBitmaps;
  bool **data;
  int transactions;
  int items;
};

#endif

