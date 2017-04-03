/****************************************************************
File Name: parameter.h  
Author: Tian Zhang, CS Dept., Univ. of Wisconsin-Madison, 1995

               Copyright(c) 1995 by Tian Zhang

                   All Rights Reserved

Permission to use, copy and modify this software must be granted
by the author and provided that the above copyright notice appear 
in all relevant copies and that both that copyright notice and this 
permission notice appear in all relevant supporting documentations. 

Comments and additions may be sent the author at zhang@cs.wisc.edu.

******************************************************************/

#ifndef PARAMETER_H
#define PARAMETER_H

#include "AttrProj.h"

class Para {
public:

short 	CorD;

int 	TotalMemSize;
int	TotalBufferSize;
int	TotalQueueSize;
int 	TotalOutlierTreeSize;

int		attrcnt;
int		attrsize;

int 		ntrees;
const int 	*attrcnts;
const int	*attrsizes;

int	total_attrcnt;
int	total_attrsize;

// for parameter reading
ifstream parafile;

// for devise-like data reading
AttrProj *attrproj;

// for logging execution information
ofstream logfile;

Para(char *pname, char *sname, char *aname, char *dname);
~Para();

friend istream& operator>>(istream &parafile,Para *Paras);
friend ifstream& operator>>(ifstream &parafile,Para *Paras);
friend ostream& operator<<(ostream &parafile,Para *Paras);
friend ofstream& operator<<(ofstream &parafile,Para *Paras);
};

istream& operator>>(istream &parafile,Para *Paras);
ifstream& operator>>(ifstream &parafile,Para *Paras);
ostream& operator<<(ostream &parafile,Para *Paras);
ofstream& operator<<(ofstream &parafile,Para *Paras);

#endif PARAMETER_H
