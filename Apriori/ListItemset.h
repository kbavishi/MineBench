#ifndef LISTITEMSET_H
#define LISTITEMSET_H

#include "Itemset.h"

//ListElement is the element in ListItemset 
class ListElement {
public:
   ListElement(Itemset *itemPtr);	// initialize a list element
   ~ListElement();
   inline ListElement *next()
   {
      return Next;
   }
   inline void set_next(ListElement *n)
   {
      Next = n;
   }
   
   inline Itemset *item()
   {
      return Item;
   }
   inline void set_item(Itemset *it)
   {
      Item = it;
   }
   
private:
   ListElement *Next;		// next element on list, 
				// NULL if this is the last
   Itemset *Item; 	    	// pointer to item on the list
};

class ListItemset {
public:
   ListItemset();			// initialize the list
   ~ListItemset();			// de-allocate the list

   void append(Itemset &item); 	// Put item at the end of the list
   Itemset *remove(); 	 	// Take item off the front of the list
   ListElement *node(int);
   void sortedInsert(Itemset *);// Put item into list
   ListElement * sortedInsert(Itemset *, ListElement *);
   void clearlist();
   friend ostream& operator << (ostream&, ListItemset&);

   inline ListElement *first()
   {
      return First;
   }

   inline ListElement *last()
   {
      return Last;
   }

   inline int numitems()
   {
      return numitem;
   }
   
private:
   ListElement *First;  	// Head of the list, NULL if list is empty
   ListElement *Last;		// Last element of list
   int numitem;
};

#endif // LISTITEMSET_H





