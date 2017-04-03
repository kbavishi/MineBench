// 	$Id: printCoOccor.cpp 2399 2014-03-13 22:43:51Z wkliao $	
#include "printCoOccor.h"

//using namespace treeInterface;
using namespace likelihoodComputation;

// added my matan - for loggin perpeses
void PrintCoOccorence(const sequenceContainer& sc,
		      const stochasticProcess& sp,
		      const Vdouble * weights,
		      const int nodeId,
		      const suffStatGlobalGam& cup,
		      const suffStatGlobalGam& cdown,
		      const MDOUBLE dist,
		      ostream& out) {

  // THIS IS USED TO compute and print the co-occorance matreces along branches

  MDOUBLE tmp = 0.0;
  VVVdouble CC(sp.categories());
  VVVdouble CC_tmp(sp.categories());
  MDOUBLE total_l=0.0;
  
  //	initialize
  for (int r = 0; r<sp.categories(); ++r) {
    CC[r].resize(sc.alphabetSize());
    for (int i=0;i<sc.alphabetSize();++i){
      CC[r][i].resize(sc.alphabetSize());
      for (int j=0;j<sc.alphabetSize();++j){
	CC[r][i][j]=0.0;
      }
    }
  }
  for (int r2 = 0; r2<sp.categories(); ++r2) {
    CC_tmp[r2].resize(sc.alphabetSize());
    for (int i=0;i<sc.alphabetSize();++i)
      CC_tmp[r2][i].resize(sc.alphabetSize());
  }





  for (int pos=0; pos < sc.seqLen(); ++pos){
    if ((weights!=NULL) && ((*weights)[pos]==0)) continue;

    // clear tmp
//     for (int r = 0; r<sp.categories(); ++r) 
//       for (int i=0;i<sc.alphabetSize();++i)
// 	for (int j=0;j<sc.alphabetSize();++j)
// 	  CC_tmp[r][i][j]=0.0;
    total_l=0.0;

    for (int alph1=0; alph1 < sc.alphabetSize(); ++alph1){
      for (int alph2=0; alph2 < sc.alphabetSize(); ++alph2){
	for (int rateCategor = 0; rateCategor<sp.categories(); ++rateCategor) {
	  tmp = sp.ratesProb(rateCategor)*
	    cup.get(pos,rateCategor,nodeId,alph1)*
	    cdown.get(pos,rateCategor,nodeId,alph2)* sp.freq(alph1);
	  CC_tmp[rateCategor][alph1][alph2]=tmp;
	  total_l+=tmp;
	}
      }
    }
    // normelize and add
    for (int r = 0; r<sp.categories(); ++r) 
      for (int i=0;i<sc.alphabetSize();++i)
	for (int j=0;j<sc.alphabetSize();++j)
	  CC[r][i][j]+=	CC_tmp[r][i][j]/total_l;
  }


  //print
  out <<"Co-Occorance for node "<<nodeId<<endl<<endl;
  for (int r3 = 0; r3<sp.categories(); ++r3) {
    out <<"rate="<<sp.rates(r3)<<endl;
    out <<"dist="<<sp.rates(r3)*dist<<endl;
    for (int i=0;i<sc.alphabetSize();++i){
      for (int j=0;j<sc.alphabetSize();++j){
	out<< CC[r3][i][j]<<" ";
      }
      out<<endl;      
    }
    out<<endl;
  }
}


void printAllCC(const sequenceContainer& sc,
		const stochasticProcess& sp,
		const Vdouble * weights,
		const tree& et,
		const tree::nodeP in_nodep,
		const suffStatGlobalGam& cup,
		const suffStatGlobalGam& cdown,
		ostream& out) {
  for(int i=0; i<in_nodep->getNumberOfSons();i++) {
      printAllCC(sc,sp,weights,et,in_nodep->getSon(i),cup,cdown, out);
  }
  if (in_nodep == et.getRoot()) return;
  PrintCoOccorence(sc,sp,weights,in_nodep->id(),cup,cdown,in_nodep->dis2father(),out);
}

