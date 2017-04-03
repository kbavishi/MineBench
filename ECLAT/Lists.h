#ifndef __LISTS_H
#define __LISTS_H

using namespace std;
#include <iostream>
#include <stdlib.h>
typedef int (*CMP_FUNC) (void *, void *);

template <class Items>
class ListNodes {
private:
   ListNodes *theNext;
   Items     theItem;

public:
   ListNodes(Items item, ListNodes *next)
   {
      theItem = item;
      theNext = next;
   }
   
   ~ListNodes(){
      theNext = NULL;
      theItem = NULL;
   }
   
   inline ListNodes<Items> *next (){
      return theNext;
   }
   
   inline void set_next (ListNodes *next){
      theNext = next;
   }

   inline Items item(){
      return theItem;
   }
   
   inline void set_item(Items item){
      theItem = item;
   }
};

template <class Items>
class Lists {
private:
   ListNodes<Items> *theHead;
   ListNodes<Items> *theLast;
   int theSize;
   
public:

   Lists(){
      theHead = 0;
      theLast = 0;
      theSize = 0;
   };

   //only listnodes are deleted, if node->item() is a pointer to some object
   //that object is *not* deleted
   ~Lists(){
      ListNodes<Items> *node = theHead;
      ListNodes<Items> *tmp;
      while (node){
         tmp = node;
         node = node->next();
         delete tmp;
      }
      theHead = NULL;
      theLast = NULL;
      theSize = 0;
   };	

   //listnodes are deleted, if node->item() is a pointer to some object
   //that object is *also*  deleted
   void clear(){
      ListNodes<Items> *node = theHead;
      ListNodes<Items> *tmp;
      while (node){
         tmp = node;
         node = node->next();
         delete tmp->item();
         delete tmp;
      }
      theHead = NULL;
      theLast = NULL;
      theSize = 0;
   };
   
   inline ListNodes<Items> *head (){
      return theHead;
   };
   inline ListNodes<Items> *last (){
      return theLast;
   };
   inline int size (){
      return theSize;
   };
   inline void set_head( ListNodes<Items> *hd)
   {
      theHead = hd;
   }
   inline void set_last( ListNodes<Items> *lst)
   {
      theLast = lst;
   }
   
   void append (Items item){
      ListNodes<Items> *node;
      
      theSize++;
      node = new ListNodes<Items> (item, 0);
      if (node == NULL){
         cout << "MEMORY EXCEEDED\n";
         exit(-1);
      }
      
      if (theHead == 0){
         theHead = node;
         theLast = node;
      }
      else{
         theLast->set_next(node);
         theLast = node;
      }
   };
   
   void remove(ListNodes<Items> * prev, ListNodes<Items> * val)
   {
      if (prev == NULL) theHead = val->next();
      else prev->set_next(val->next());
      if (theLast == val) theLast = prev;
      theSize--;
   }
   
   void sortedDescend(Items item, CMP_FUNC cmpare){
      ListNodes<Items> *node;
      ListNodes<Items> *temp = theHead;
      
      //printf("theSize %d\b", theSize);
      theSize++;
      node = new ListNodes<Items>(item, 0);
      if (node == NULL){
         cout << "MEMORY EXCEEDED\n";
         exit(-1);
      }
      
      if (theHead == 0){
         theHead = node;
         theLast = node;
      }
      else if (cmpare((void *)item,(void *)theHead->item()) > 0){
         node->set_next(theHead);
         theHead = node;
      }
      else{
         while (temp->next()){
            if (cmpare((void *)item,(void *)temp->next()->item()) > 0){
               node->set_next(temp->next());
               temp->set_next(node);
               return;
            }
            temp = temp->next();
         }
         theLast->set_next(node);
         theLast = node;
      } 
   };
   
   
   void sortedAscend (Items item, CMP_FUNC cmpare) 
   {
      ListNodes<Items> *node;
      ListNodes<Items> *temp = theHead;

      theSize++;
      node = new ListNodes<Items>(item,0);
      if (node == NULL){
         cout << "MEMORY EXCEEDED\n";
         exit(-1);
      }

      if (theHead == 0){
         theHead = node;
         theLast = node;
      }
      else if (cmpare((void *)item,(void *)theHead->item()) < 0){
         node->set_next(theHead);
         theHead = node;
      }
      else{
         while (temp->next()){
            if (cmpare((void *)item,(void *)temp->next()->item()) < 0){
               node->set_next(temp->next());
               temp->set_next(node);
               return;
            }
            temp = temp->next();
         }
         theLast->set_next(node);
         theLast = node;
      }
   };
   
    void print(){
//       cout << "LIST: ";
//       if (!(theHead == NULL)){
//          ListNodes<Items> *head = theHead;
//          for (; head; head = head->next()){
//             cout << *(head->item()) << " ";
//          }
//       }
    };

   void remove_ascend(Items val, CMP_FUNC cmpare);
   ListNodes<Items> *node(int pos);

   Items find(Items item, CMP_FUNC cmpare)
   {
      ListNodes<Items> *temp = theHead;
      for (;temp; temp = temp->next())
         if (cmpare((void *)item,(void *)temp->item()) == 0)
            return temp->item();
      return NULL;
   }
   
   int find_ascend(ListNodes<Items> *&prev,
                                 Items item, CMP_FUNC cmpare)
   {
      ListNodes<Items> *temp = theHead;
      if (theHead == 0) return 0;
      else{
         int res = cmpare((void *)item,(void *)theHead->item());
         if (res == 0) return 1;
         else if (res < 0) return 0;
         else{
            while (temp->next()){
               res = cmpare((void *)item,(void *)temp->next()->item());
               if (res < 0){
                  prev = temp;
                  return 0;
               }
               else if (res == 0){
                  prev = temp;
                  return 1;
               }
               temp = temp->next();
            }
            prev = theLast;
            return 0;
         }
      }      
   }

   void prepend (Items item)
      {
         
         theSize++;
         ListNodes<Items> *node = new ListNodes<Items> (item, 0);
         if (node == NULL){
            cout << "MEMORY EXCEEDED\n";
            exit(-1);
         }
         if (theHead == NULL){
            theHead = node;
            theLast = node;
         }
         else{
            node->set_next(theHead);
            theHead = node;
         }
      }
   

   void insert(ListNodes<Items> *&prev, Items item)
   {
      theSize++;
      ListNodes<Items> *node = new ListNodes<Items>(item,0);
      if (node == NULL){
         cout << "MEMORY EXCEEDED\n";
         exit(-1);
      }

      if (prev == NULL){
         theHead = node;
         theLast = node;
      }
      else{
         node->set_next(prev->next());
         prev->set_next(node);
         if (prev == theLast) theLast = node;
      }  
   }
   
};

#endif// __LISTS_H


