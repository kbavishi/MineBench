/****************************************************************
File Name: path.C 
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#include "global.h"
#include "util.h"
#include "vector.h"
#include "rectangle.h"
#include "parameter.h"
#include "cfentry.h"
#include "status.h"
#include "path.h"
#include "cftree.h"
#include "cutil.h"

Path::Path(int size) {
	height = size;
	stacktop = -1;
	indexstack = new int[size];
	nodestack = new Node*[size];
	}

void Path::Push(int index, Node* node) {
	stacktop++;
	indexstack[stacktop]=index;
	nodestack[stacktop]=node;
	}

void Path::Pop(int &index, Node** node) {
	index=indexstack[stacktop];
	*node=nodestack[stacktop];
	stacktop--;
	}

Entry* Path::TopLeafEntry() const {
if (stacktop==height-1)
	return nodestack[stacktop]->TheEntry(indexstack[stacktop]);
else return NULL;
}

Node* Path::TopLeaf() const {
if (stacktop==height-1) 
	return nodestack[stacktop];
else return NULL;
}

void Path::operator=(const Path& path) 
{ height = path.height;
  stacktop = path.stacktop;
  memcpy(indexstack, path.indexstack, height*sizeof(int));
  memcpy(nodestack, path.nodestack, height*sizeof(Node *));
  }

short Path::operator==(const Path& path)
{ if (height!=path.height) return FALSE;
  if (stacktop!=path.stacktop) return FALSE;
  for (int i=0; i<height; i++) 
	if (indexstack[i]!=path.indexstack[i])
	    return FALSE;
  return TRUE;
}

short Path::operator>(const Path& path)
{ for (int i=0; i<height; i++) { 
	if (indexstack[i]>path.indexstack[i]) 
		return TRUE;
	if (indexstack[i]<path.indexstack[i])
		return FALSE;
	}	
  return FALSE;
}

short Path::operator<(const Path& path)
{ for (int i=0; i<height; i++) {
	if (indexstack[i]<path.indexstack[i])
		return TRUE;
        if (indexstack[i]>path.indexstack[i])
		return FALSE;
	}
  return FALSE;
}

short Path::NextRightPath() 
{
while (stacktop>=0 && 
       indexstack[stacktop]+1>=nodestack[stacktop]->actsize) 
	stacktop--;

if (stacktop<0) return FALSE;
else {
   indexstack[stacktop]++;
   while (stacktop<height-1) {
	nodestack[stacktop+1]=nodestack[stacktop]->TheChild(indexstack[stacktop]);
	indexstack[stacktop+1]=0;
	stacktop++;
	}
   return TRUE;
   }
}

Node* Path::NextRightLeafFreeSpace(Stat *Stats)
{

delete nodestack[stacktop]; 
Stats->MemUsed--; 
stacktop--;

while (stacktop>=0 && 
       indexstack[stacktop]+1>=nodestack[stacktop]->actsize) {
	    delete nodestack[stacktop]; 
	    Stats->MemUsed--; 
	    stacktop--;
	    }

if (stacktop<0) return NULL;

else {
   indexstack[stacktop]++;
   while (stacktop<height-1) {
	nodestack[stacktop+1]=
	    nodestack[stacktop]->TheChild(indexstack[stacktop]);
	indexstack[stacktop+1]=0;
	stacktop++;
	}
   return nodestack[stacktop];
   }
}

short Path::CollectSpace(Stat *Stats)
{
nodestack[stacktop]->DeleteEntry(indexstack[stacktop]);

while (stacktop>0 && nodestack[stacktop]->actsize==0) {
	nodestack[stacktop]->AssignNextPrev(Stats);
    	delete nodestack[stacktop]; 
	Stats->MemUsed--; 
	Stats->TreeSize--;
    	stacktop--;
    	nodestack[stacktop]->DeleteEntry(indexstack[stacktop]);
    	}

if (indexstack[stacktop]<nodestack[stacktop]->actsize) {
   while (stacktop<height-1) {
	nodestack[stacktop+1]=
		nodestack[stacktop]->TheChild(indexstack[stacktop]);
	indexstack[stacktop+1]=0;
	stacktop++;
	}
   return TRUE;
   }

else { 
   while (stacktop>=0 && 
	  indexstack[stacktop]+1>=nodestack[stacktop]->actsize)
	  stacktop--;
   if (stacktop==-1) return FALSE;
   else {
	if (indexstack[stacktop]+1<nodestack[stacktop]->actsize) {
	   indexstack[stacktop]++;
           while (stacktop<height-1) {
		nodestack[stacktop+1]=nodestack[stacktop]->TheChild(indexstack[stacktop]);
		indexstack[stacktop+1]=0;
		stacktop++;
		}
	   return TRUE;
	   }
        else return FALSE;
        }
   }
}

void Path::TakeoffPath(const Entry &ent)
{
for (int i=0; i<height; i++) 
	nodestack[i]->SubEntry(indexstack[i],ent);
}

void Path::InsertLeaf(Stat *Stats, Node *Root)
{
int i;
Node *tmpnode=Root;

for (i=0; i<height; i++) {
	if (indexstack[i]>=tmpnode->actsize) break;
	else tmpnode=tmpnode->TheChild(indexstack[i]);
	}

tmpnode->actsize++;
while (i<height-2) {
	tmpnode->NewNonleafChildI(Stats,indexstack[i]);
	Stats->MemUsed++; Stats->TreeSize++;
	tmpnode=tmpnode->TheChild(indexstack[i]);
	tmpnode->actsize=1;
	i++;
	}

tmpnode->NewLeafChildI(Stats,indexstack[i]);
Stats->MemUsed++; Stats->TreeSize++;
tmpnode=tmpnode->TheChild(indexstack[i]);
tmpnode->actsize=0;
tmpnode->ChainNextPrev(Stats);
}

Path::~Path() {
	if (indexstack!=NULL) delete [] indexstack;
	if (nodestack!=NULL) delete [] nodestack;
	}

ostream& operator<<(ostream &fo,const Path& path)
{ 
int i;
for (i=0; i<path.height; i++) 
	fo << path.indexstack[i] << "\t";
fo << endl;
return fo;
}
ofstream& operator<<(ofstream &fo,const Path& path)
{ 
int i;
for (i=0; i<path.height; i++) 
	fo << path.indexstack[i] << "\t";
fo << endl;
return fo;
}

void Path::AddonPath(Stat *Stats, const Entry &ent, Node *Root)
{
Node *tmpnode=Root;
int i;
for (i=0; i<height-1; i++) {
	tmpnode->AddEntry(indexstack[i],ent);
	tmpnode=tmpnode->TheChild(indexstack[i]);
	}
if (indexstack[height-1]<tmpnode->actsize) {
	tmpnode->AddEntry(indexstack[height-1],ent);
	}
else {
	Stats->CurrEntryCnt++;
	tmpnode->AttachEntry(ent,NULL);
	}
}

void Path::AddonLeaf(Stat *Stats, const Entry &ent, Node *Root)
{
Node *tmpnode=Root;
int i;
for (i=0; i<height-1; i++) {
	tmpnode->AddEntry(indexstack[i],ent);
	tmpnode=tmpnode->TheChild(indexstack[i]);
	}

Stats->CurrEntryCnt++;
tmpnode->AttachEntry(ent,NULL);
}


