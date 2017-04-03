#include "DataSet.h"
#include <algorithm>
#include <vector>

bool operator<(const Bitmap &a, const Bitmap &b)
{
  return (a.CountOnes() < b.CountOnes());
}

bool operator==(const Bitmap &a, const Bitmap &b)
{
  bool equal = (a.CountOnes() == b.CountOnes());
  return equal;
}

// load in a file with market-basket data (1-based indexing)
DataSet::DataSet(string filename)
{
  ifstream inFile(filename.c_str());
  items = 0;
  
  // load the data from the file
  string thisLine;    
  while (getline(inFile, thisLine)) {
    vector <int> currentLine;
    int temp;
    stringstream stream(thisLine, stringstream::in);
    while (stream >> temp) {
      currentLine.push_back(temp);
  
      if (temp > items)
	items = temp;
    }
    theData.push_back(currentLine);
  }
  transactions = theData.size(); 

  // create the vectors of bitmaps for both rows (transactions) and columns (items)
  for (int item = 0; item < items; item++) {
    itemBits.push_back(Bitmap(transactions));
    itemBits[item].SetID(item+1);
    itemBits[item].Clear();
  }
  for (int trans = 0; trans < transactions; trans++) {
    transBits.push_back(Bitmap(items));
    transBits[trans].Clear();
  }  
  
  // fill the item bitmaps
  for (int trans = 0; trans < transactions; trans++) {    
    int size = (int) theData[trans].size();
    for (int item = 0; item < size; item++) {
      itemBits[theData[trans][item]-1].SetBit(trans, true);      
    }
  }  

  // sort the items in order of increasing support
  vector<Bitmap>::iterator start = itemBits.begin();
  vector<Bitmap>::iterator end = itemBits.end();
  sort(start, end);

  // fill the transaction bitmaps
  for (int trans = 0; trans < transactions; trans++) {
    for (int item = 0; item < items; item++) {      
      transBits[trans].SetBit(item, itemBits[item].GetBit(trans));
    }
  }

  // sort the transactions in order of increasing width
  /* start = transBits.begin();
  end = transBits.end();
  sort(start, end);*/

  //  cerr << "loaded " << transactions << " transactions, "
  //   << items << " items" << endl;
}

Bitmap * DataSet::GetItemBitmap(int index)
{
  return &(itemBits[index]);
}

//Bitmap * DataSet::GetBitmaps() const {return itemBits;}
int DataSet::GetActualIndex(int index) const
{
  return itemBits[index].GetID();
}


void DataSet::AddItemToZeroCount(int *zeros, int item) const
{
  for (int r = 0; r < transactions; r++) {
    if (!ValueAt(r, item)) {
      zeros[r]++;
      //cout << "zero at: " << r << ", " << item << endl;
    }
  }
}

void DataSet::RemoveItemFromZeroCount(int *zeros, int item) const
{
  for (int r = 0; r < transactions; r++)
    if (!ValueAt(r, item))
      zeros[r]--;
}

int DataSet::GetRemainingOnesInTransaction(int trans, int after) const
{
  return transBits[trans].CountOnesAfter(after);
}

int * DataSet::GetSupportSet(int searchFor, int *length) 
{
  vector<int> indices;

  for (int i = 0; i < transactions; i++) {
    if (ValueAt(i, searchFor))
      indices.push_back(i);
  }

  *length = indices.size();
  int *newSet = new int[*length];
  for (int i = 0; i < *length; i++)
    newSet[i] = indices[i];

  return newSet;
}

bool DataSet::ValueAt(int r, int c) const
{    
  bool item = itemBits[c].GetBit(r);
  bool trans = transBits[r].GetBit(c);
  
  if (item != trans)
    cerr << "oh no!!!" << endl;

  return itemBits[c].GetBit(r);//(supportBitmaps[r / 32][c] & (1 << (r % 32)));// << endl;
      
  int lineSize = theData[r].size();
  
  int left = 0;
  int right = lineSize - 1;
  int mid;
  int searchingFor = c + 1;
  
  if (r >= transactions)
    cout << "r value too high: " << r << endl;
  if (c > items)
    cout << "c value too high: " << c << endl;

  while (left < right) {
    // cout << left << ", " << right << endl;
    mid = (right + left) / 2;
    
    if (theData[r][mid] < searchingFor) {
      left = mid + 1;
    }
    else if (theData[r][mid] > searchingFor) {
      right = mid - 1;
    }
    else if (theData[r][mid] == searchingFor) {
      return true;
    }
    
    /*
    if (left == right - 1)
      return (theData[r][left] == searchingFor ||
      theData[r][right] == searchingFor);*/
  }

  return false;
}
int DataSet::GetTransactionCount() const {return transactions;}
int DataSet::GetItemCount() const {return items;}

ostream & operator<<(ostream & out, DataSet & d)
{  
  for (int item = 0; item < d.items; item++) {
    for (int row = 0; row < d.transactions; row++) {    
      out << (d.ValueAt(row, item) ? "X" : ".");
    }
    out << " " << d.itemBits[item].GetID() << " - "
	<< d.itemBits[item].CountOnes() << endl;
  }
  
  for (int row = 0; row < d.transactions; row++) {
    out << d.transBits[row].CountOnes() << endl;
  }

  return out;
}
