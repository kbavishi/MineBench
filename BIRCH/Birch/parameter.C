/****************************************************************
File Name: parameter.C   
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
#include "parameter.h"

Para::Para(char *pname, char *sname, char *aname, char *dname) {
	char logname[100];

	parafile.open(pname);

	attrproj=new AttrProj(sname,aname,dname);

	attrproj->GetWholeRecSize(attrcnt,attrsize);

	attrproj->GetDataSize(ntrees,attrcnts,attrsizes);
	total_attrcnt=0;
	total_attrsize=0;
	for (int i=0; i<ntrees; i++) {
		total_attrcnt+=attrcnts[i];
		total_attrsize+=attrsizes[i];
		}

	sprintf(logname,"birch.log.out");
	logfile.open(logname);
	}


Para::~Para() {
	parafile.close();
	delete attrproj;
	logfile.close();
	}

istream& operator>>(istream &fi,Para *Paras) {

fi>>Paras->CorD;
fi>>Paras->TotalMemSize;
fi>>Paras->TotalBufferSize;
fi>>Paras->TotalQueueSize;
fi>>Paras->TotalOutlierTreeSize;

return fi;
}

ifstream& operator>>(ifstream &fi,Para *Paras) {

fi>>Paras->CorD;
fi>>Paras->TotalMemSize;
fi>>Paras->TotalBufferSize;
fi>>Paras->TotalQueueSize;
fi>>Paras->TotalOutlierTreeSize;

return fi;
}

ostream& operator<<(ostream &fo,Para *Paras) {
fo<<"Global Parameters:\n";
fo<<"CorD\t"<<Paras->CorD<<endl;
fo<<"TotalMemSize\t"<<Paras->TotalMemSize<<endl;
fo<<"TotalBufferSize\t"<<Paras->TotalBufferSize<<endl;
fo<<"TotalQueueSize\t"<<Paras->TotalQueueSize<<endl;
fo<<"TotalOutlierTreeSize\t"<<Paras->TotalOutlierTreeSize<<endl;
fo<<"ntrees\t"<<Paras->ntrees<<endl;
for (int i=0; i<Paras->ntrees; i++)
	fo<<Paras->attrcnts[i]<<" "<<Paras->attrsizes[i]<<"\t";
fo<<endl;
fo<<"total_attrcnt\t"<<Paras->total_attrcnt<<endl;
fo<<"total_attrsize\t"<<Paras->total_attrsize<<endl;
return fo;
}

ofstream& operator<<(ofstream &fo,Para *Paras) {
fo<<"Global Parameters:\n";
fo<<"CorD\t"<<Paras->CorD<<endl;
fo<<"TotalMemSize\t"<<Paras->TotalMemSize<<endl;
fo<<"TotalBufferSize\t"<<Paras->TotalBufferSize<<endl;
fo<<"TotalQueueSize\t"<<Paras->TotalQueueSize<<endl;
fo<<"TotalOutlierTreeSize\t"<<Paras->TotalOutlierTreeSize<<endl;
fo<<"ntrees\t"<<Paras->ntrees<<endl;
for (int i=0; i<Paras->ntrees; i++)
	fo<<Paras->attrcnts[i]<<" "<<Paras->attrsizes[i]<<"\t";
fo<<endl;
fo<<"total_attrcnt\t"<<Paras->total_attrcnt<<endl;
fo<<"total_attrsize\t"<<Paras->total_attrsize<<endl;
return fo;
}

