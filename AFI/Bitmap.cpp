#include "Bitmap.h"
#include <iomanip>

int Bitmap::oneCount[256] = 
{0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

Bitmap::Bitmap() {}

Bitmap::Bitmap(int bits):
  numberOfBits(bits), numberOfBytes((int) ceil((double)bits / 8))
{
  column = new char[numberOfBytes];
  for (int i = 0; i < numberOfBytes; i++)
    column[i] = 0;
}

int Bitmap::GetID() const {return id;}
void Bitmap::SetID(int id) {this->id = id;}
/*bool Bitmap::operator<(const Bitmap &b)
{
  return (a.CountOnes() < b.CountOnes());
}*/

int Bitmap::CountOnes() const
{
  int count = 0;
  for (int i = 0; i < numberOfBytes; i++) {
    count += (unsigned int) oneCount[((unsigned int) column[i]) & 0xFF];   
  }

  return count;
}

int Bitmap::CountOnesAfter(int after) const
{
  int firstByte = (after + 1) / 8;
  int count = 0;
  for (int i = firstByte; i < numberOfBytes; i++) {
    count += (unsigned int) oneCount[((unsigned int) column[i]) & 0xFF];   
  }

  int extraBits = (after + 1) % 8;
  for (int i = firstByte * 8; i < firstByte * 8 + extraBits; i++) {
    if (GetBit(i))
      count--;
  }

  /*  int count2 = 0;
  for (int i = after + 1; i < numberOfBits; i++)
    if (GetBit(i))
      count2++;

  if (count != count2)
    cerr << "something went very wrong" << endl;
  */
  return count;
}

void Bitmap::Clear()
{
  for (int i = 0; i < numberOfBytes; i++)
    column[i] = 0;
}

void Bitmap::SetBit(int bitNumber, bool value)
{
  if (value) {
    column[bitNumber / 8] |= (1 << (bitNumber % 8));
  }
  else {
    column[bitNumber / 8] &= ~(1 << (bitNumber % 8));
  }
}

bool Bitmap::GetBit(int bitNumber) const
{
  return ((column[bitNumber / 8] & (1 << (bitNumber % 8))));
}

int Bitmap::LastOne() const
{
  for (int byte = numberOfBytes - 1; byte >= 0; byte--)
    if (column[byte])
      for (int bit = 7; bit >= 0; bit--)
	if (column[byte] & (1 << bit))
	  return byte * 8 + bit;

  return -1;
}

double Bitmap::Cosine(Bitmap *lhs, Bitmap *rhs)
{
  int numerator = 0;
  
  for (int bit = 0; bit < lhs->numberOfBits; bit++)
    if (lhs->GetBit(bit) && rhs->GetBit(bit))
      numerator++;

  int denominator = lhs->CountOnes() * rhs->CountOnes();

  return (double) numerator / denominator;
}

double Bitmap::Correlation(Bitmap *lhs, Bitmap *rhs)
{
  int zeroZero = 0;
  int zeroOne = 0;
  int oneZero = 0;
  int oneOne = 0;  
  
  for (int bit = 0; bit < lhs->numberOfBits; bit++) {
    if (lhs->GetBit(bit) && rhs->GetBit(bit))
      oneOne++;
    else if (lhs->GetBit(bit) && !rhs->GetBit(bit))
      oneZero++;
    else if (!lhs->GetBit(bit) && rhs->GetBit(bit))
      zeroOne++;
    else
      zeroZero++;
  }

  int numerator = oneOne * zeroZero - zeroOne * oneZero;
  int denominator = (oneOne + oneZero) * (zeroOne + oneOne) *
    (zeroOne + zeroZero) * (oneZero + zeroZero);

  return (double) numerator / sqrt(denominator);
}

bool Bitmap::ContainedIn(Bitmap *rhs) const
{
  for (int bit = 0; bit < numberOfBits; bit++) 
    if (GetBit(bit) && !rhs->GetBit(bit)) {
      //      cout << "failed at bit " << bit << endl;
      return false;
    }
  
  return true;
}

Bitmap & Bitmap::operator&=(const Bitmap &rhs)
{
  if (rhs.numberOfBits != numberOfBits) {
    cerr << "mismatched bitmaps" << endl;
    return *this;
  }
  else {
    for (int i = 0; i < numberOfBytes; i++)
      column[i] &= rhs.column[i];
    return *this;
  }
}

bool Bitmap::operator==(const Bitmap &rhs)
{
  if (this == &rhs)
    return true;

  if (numberOfBits != rhs.numberOfBits)
    return false;

  for (int i = 0; i < numberOfBytes; i++)
    if (column[i] != rhs.column[i])
      return false;

  return true;
}

Bitmap & Bitmap::operator=(const Bitmap &rhs)
{
  if (this == &rhs)
    return *this;

  numberOfBits = rhs.numberOfBits;
  numberOfBytes = rhs.numberOfBytes;
  id = rhs.id;

  column = new char[numberOfBytes];
  memcpy(column, rhs.column, numberOfBytes);

  return *this;
}

Bitmap & Bitmap::operator|(const Bitmap &rhs)
{
  Bitmap *temp = new Bitmap(rhs.numberOfBits);
  if (numberOfBits != rhs.numberOfBits) {
    cerr << "mismatched bitmaps" << endl;
    return *temp;
  }
  else {    
    for (int i = 0; i < numberOfBytes; i++)
      temp->column[i] = column[i] | rhs.column[i];
    return *temp;
  }
}

Bitmap & Bitmap::operator&(const Bitmap &rhs)
{
  Bitmap *temp = new Bitmap(rhs.numberOfBits);
  if (numberOfBits != rhs.numberOfBits) {
    cerr << "mismatched bitmaps" << endl;
    return *temp;
  }
  else {    
    for (int i = 0; i < numberOfBytes; i++)
      temp->column[i] = column[i] & rhs.column[i];
    return *temp;
  }
}

Bitmap & Bitmap::operator|=(const Bitmap &rhs)
{
  if (rhs.numberOfBits != numberOfBits) {
    cerr << "mismatched bitmaps" << endl;
    return *this;
  }
  else {
    for (int i = 0; i < numberOfBytes; i++)
      column[i] &= rhs.column[i];
    return *this;
  }
}

ostream & operator<<(ostream & out, Bitmap & bm)
{
  for (int byte = 0; byte < bm.numberOfBytes; byte++) {
    for (int bit = 0; bit < 8; bit++) {
      out << ((bm.column[byte] & (1 << bit)) ? "X" : ".");
    }
  }
  out << endl;

  return out;
}
  
  
