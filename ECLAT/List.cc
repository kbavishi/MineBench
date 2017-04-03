#include "Lists.h"

template <class Items> 
void ListNodes<Items>::clear(){
   theNext = NULL;
   delete theItem;
} 

void ListNodes<int>::clear(){
   theNext = NULL;
   theItem = 0;
} 

template <class Items> 
Lists<Items>::Lists() 
{
   theHead = 0;
   theLast = 0;
   theSize = 0;
}

template <class Items> 
Lists<Items>::~Lists() 
{
   clear();
}

template <class Items>
void Lists<Items>::clear()
{
   ListNodes<Items> *node = theHead;
   
   while (node){
      theHead = theHead->theNext;
      node->clear();
      delete node;
      node = theHead;
   }
   theHead = NULL;
   theLast = NULL;
   theSize = 0;
}

template <class Items> 
void Lists<Items>::remove(ListNodes<Items> * prev, ListNodes<Items> * val)
{
   if (prev == NULL) theHead = val->theNext;
   else prev->theNext = val->theNext;
   theSize--;
}

template <class Items> 
void Lists<Items>::remove_ascend(Items val, CMP_FUNC cmpare)
{
   ListNodes<Items> *prev = NULL;
   if (find_ascend(prev, val, cmpare)){
      if (prev == NULL) remove (prev, theHead);
      else remove (prev, prev->theNext);
   }
}


template <class Items> 
int Lists<Items>::find_ascend(ListNodes<Items> *&prev,
                             Items item, CMP_FUNC cmpare)
{
   ListNodes<Items> *temp = theHead;
   if (theHead == 0) return 0;
   else{
      int res = cmpare((void *)item,(void *)theHead->theItem);
      if (res == 0) return 1;
      else if (res < 0) return 0;
      else{
         while (temp->theNext){
            res = cmpare((void *)item,(void *)temp->theNext->theItem);
            if (res < 0){
               prev = temp;
               return 0;
            }
            else if (res == 0){
               prev = temp;
               return 1;
            }
            temp = temp->theNext;
         }
         prev = theLast;
         return 0;
      }
   }      
}
//find_descend(ListNodes<Items> *&, Items, CMP_FUNC);


template <class Items> 
void Lists<Items>::append (Items item) 
{
   ListNodes<Items> *node;
   
   theSize++;
   node = new (ListNodes<Items>);
   if (node == NULL){
      cout << "MEMORY EXCEEDED\n";
      exit(-1);
   }
   node->theItem = item;
   node-> theNext = 0;
   
   if (theHead == 0){
      theHead = node;
      theLast = node;
   }
   else{
      theLast->theNext = node;
      theLast = node;
   }
}

template <class Items>
ListNodes<Items> *Lists<Items>::node(int pos){
   ListNodes<Items> *head = theHead;
   for (int i=0; i < pos && head; head = head->theNext,i++);
   return head;
}


template <class Items> 
void Lists<Items>::sortedAscend (Items item, CMP_FUNC cmpare) 
{
   ListNodes<Items> *node;
   ListNodes<Items> *prev = NULL;
   
   if (!find_ascend(prev, item, cmpare)){
      node = new (ListNodes<Items>);
      if (node == NULL){
         cout << "MEMORY EXCEEDED\n";
         exit(-1);
      }
      node->theItem = item;
      node->theNext = 0;
      theSize++;
      
      if (theHead == 0){
         theHead = node;
         theLast = node;
      }
      else if (prev == NULL){
         node->theNext = theHead;
         theHead = node;         
      }      
      else{
         node->theNext = prev->theNext;
         prev->theNext = node;
         if (prev == theLast) theLast = node;
      }
   }
}

template <class Items> 
void Lists<Items>::sortedDescend (Items item, CMP_FUNC cmpare) 
{
   ListNodes<Items> *node;
   ListNodes<Items> *temp = theHead;
   
   /*printf("theSize %d\b", theSize);*/
   theSize++;
   node = new ListNodes<Items>;
   if (node == NULL){
      cout << "MEMORY EXCEEDED\n";
      exit(-1);
   }
   node->theItem = item;
   node->theNext = 0;
   
   if (theHead == 0){
      theHead = node;
      theLast = node;
   }
   else if (cmpare((void *)item,(void *)theHead->theItem) > 0){
      node->theNext = theHead;
      theHead = node;
   }
   else{
      while (temp->theNext){
         if (cmpare((void *)item,(void *)temp->theNext->theItem) > 0){
            node->theNext = temp->theNext;
            temp->theNext = node;
            return;
         }
         temp = temp->theNext;
      }
      theLast->theNext = node;
      theLast = node;
   }
   
}

void Lists<int>::print (){
   cout << "LIST: ";
   if (!(theHead == NULL)){
      ListNodes<int> *head = theHead;
      for (; head; head = head->theNext){
         cout << head->theItem << " ";
      }
      cout << "\n";
   }
}

template <class Items>
void Lists<Items>::print (){
   cout << "LIST: ";
   if (!(theHead == NULL)){
      ListNodes<Items> *head = theHead;
      for (; head; head = head->theNext){
         cout << *(head->theItem) << " ";
      }
   }
}

template <class Items>
Items Lists<Items>::Frontremove (){ 
   ListNodes<Items> *node = theHead;
   if (theHead){
      Items it = theHead->theItem;
      theHead = node->theNext;
      delete node;
      return it;
   }
   else return NULL;
}

