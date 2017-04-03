#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "Eqclass.h"

void eq_print(Eqclass *LargeEqclass){
   ListNodes<Itemset*> *head = LargeEqclass->list()->head();
   if (head){
      cout << (*head->item())[0] << ": ";
      while(head){
         cout << (*head->item())[1] << " ";
         head = head->next();
      }
      cout << endl;
   }
}
// void eq_print(Lists<Eqclass *> *LargeEqclass){
//    ListNodes<Eqclass *> *head = LargeEqclass->head();
//    while(head){
//       cout << *(head->item()->prefix());
//       ListNodes<int> *ll = head->item()->class_els()->head();
//       while(ll){
//          cout << ll->item() << " ";
//          ll = ll->next();
//       }
//       cout << endl;
//       head = head->next();
//    }
// }

Eqclass * get_node(Itemset *it){
   Eqclass *node = new Eqclass();
   if (node == NULL){
      cout << "MEMORY EXCEEDED\n";
      exit(-1);
   }
   node->list()->append(it);
   return node;
}

void eq_insert(Lists<Eqclass *> &EQC, Itemset *it){
   ListNodes<Eqclass *> *head = EQC.head();
   ListNodes<Eqclass *> *lnode;
   
   if (head == NULL){
      lnode = new ListNodes<Eqclass *>(get_node(it), 0);
      if (lnode == NULL){
         cout << "MEMORY EXCEEDED\n";
         exit(-1);
      }
      EQC.set_head(lnode);
      EQC.set_last(lnode);
   }
   else{
      int res  = it->compare(*(head->item()->list()->head()->item()),it->size()-1);
      if (res == 0){
         head->item()->sortedAscend(it, Itemset::Itemcompare);        
      }
      else if (res < 0){
         lnode = new ListNodes<Eqclass *>(get_node(it),0);
         if (lnode == NULL){
            cout << "MEMORY EXCEEDED\n";
            exit(-1);
         }
         lnode->set_next(EQC.head());
         EQC.set_head(lnode);
      }
      else{
         for (;head->next(); head = head->next()){
            res  = it->compare(*(head->next()->item()->list()->head()->item()),
                               it->size()-1);
            if (res == 0){
               head->next()->item()->sortedAscend(it, Itemset::Itemcompare);
               return;
            }
            else if ( res < 0){
               lnode = new ListNodes<Eqclass *>(get_node(it),0);
               if (lnode == NULL){
                  cout << "MEMORY EXCEEDED\n";
                  exit(-1);
               }
               lnode->set_next(head->next());
               head->set_next(lnode);
               return;
            }
         }
         lnode = new ListNodes<Eqclass *>(get_node(it),0);
         if (lnode == NULL){
            cout << "MEMORY EXCEEDED\n";
            exit(-1);
         }
         EQC.last()->set_next(lnode);
         EQC.set_last(lnode);
      }     
   }
}





