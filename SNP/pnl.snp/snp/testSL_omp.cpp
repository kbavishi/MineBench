/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AStructureLearning.cpp                                      //
//                                                                         //
//  Purpose:   Test on structure learning of BNet                          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <stdio.h>
#include "pnlMlStaticStructLearn.hpp"
#include "pnlMlStaticStructLearnHC.hpp"
// #include "wtime.h"

// #define _OPT
#ifdef _MPI
#include "mpiwrap.h"
#endif

#ifdef _OPT
#define ALIGN_BYTES		64

char *alignMalloc( int len  )
{
	char *oriP = (char *)calloc(len + ALIGN_BYTES, sizeof (char));
	char *alignP = oriP + ALIGN_BYTES - ((long)oriP & (ALIGN_BYTES-1)); 
	((void **)alignP)[-1] = oriP;
	return alignP;
}

void alignFree(void * p)
{
	free(((void **)p)[-1]);
}

#endif
PNL_USING
CBNet* CreateSNPNet(const int nnodes, int maxcpd)
{
	const int numberOfNodeTypes = nnodes ;
	
	int i;

	CGraph *pGraph = CGraph::Create(0,NULL,NULL,NULL);

	pGraph->AddNodes( nnodes );

	CNodeType *nodeTypes = new CNodeType [numberOfNodeTypes];
	for ( i = 0 ; i < numberOfNodeTypes; i++ )
	{
		nodeTypes[i].SetType(1,4);
	}
		
	int *nodeTypeAssociation = new int [nnodes];
	for ( i = 0; i < nnodes; i++ )
	{
		nodeTypeAssociation[i] = i;
	}

	//the graph model built over, the following will build the CPD and BN

	CBNet *pBNet = CBNet::Create( nnodes, numberOfNodeTypes, nodeTypes, nodeTypeAssociation, pGraph);

	CModelDomain *pMD = pBNet->GetModelDomain();

	CFactor **myParams = new CFactor* [nnodes];

	
	int **domain = new int *[nnodes] ;
	int *domainpoint1 = new int [1];
	int *domainpoint;
	domain[0] = domainpoint1;
	domainpoint1[0] = 0;
	for ( i = 1; i < nnodes; i++)
	{
		domainpoint = new int [1];
		domainpoint[0] = i;
		domain[i] = domainpoint;
	}

	int *nodeNumbers = new int[nnodes];
	nodeNumbers[0] = 1;
	for ( i = 1; i < nnodes; i++ )
	{
		nodeNumbers[i] =  1;
	}

	pBNet->AllocParameters();

	for( i = 0; i < nnodes; i++ )
	{
		myParams[i] = CTabularCPD::Create( domain[i], nodeNumbers[i], pMD);
	}

	// floatVector data;
	// data.assign(maxcpd, 0.0f);

	float *data = new float[maxcpd];
	for (i = 0; i < maxcpd; i++)
		data[i] = 0.0f;
	
	for ( i = 0; i < nnodes; i++ )
	{
		// myParams[i]->AllocMatrix(data.begin(), matTable);
		myParams[i]->AllocMatrix(data, matTable);
		pBNet->AttachParameter(myParams[i]);
	}

	delete [] nodeTypes;
	for(i=0; i<nnodes; i++)
	{
		delete domain[i];
	}
	delete[] domain;
	delete nodeNumbers;
	return pBNet;
}

int main(int argc, char *argv[])
{
	//timer_start(0);

	int i, j;
	const int nnodes = 48 ;
	int numNt = nnodes;
	CBNet *bNet = CreateSNPNet(nnodes, 2000);
	
	int nEvk;

	int* obs_nodes = new int[nnodes];
	for(i=0; i<nnodes; i++)obs_nodes[i] = i;
	char *sampleper;
	char **pEvidencesk = new char *[620000];

    /****
	if ( argc < 2 )
	{
		printf("\nUsage %s data_file_name", argv[0]);
		return -1;
	}
	*******/
	
	FILE *fp;
	fp = fopen( argv[1], "r" );
	//fp = fopen( "c:\\clai\\snpsc\\snp_s_ar", "r" );
	
	if( !fp )
	{
		std::cout<<"can't open cases file"<< argv[1] << std::endl;
		exit(1);
	}
	int knum = 0;
	//while(( !feof(fp) ) && (knum < 60000))
	while( !feof(fp) )
	{
		sampleper = new char [51];
		fscanf(fp, "%s", sampleper);
		pEvidencesk[knum] = sampleper;
		knum = knum +1;
	}
	nEvk = knum;
	fclose(fp);
	int nEv = 0;
	int *samplelable = new int [nEvk];	
	for (i = 0; i < nEvk; i++)
	{
		samplelable[i] = 0;
		for (j = 1; j < 49; j++)	//25
		{
			switch (pEvidencesk[i][j])
			{
				case 'A': 
					pEvidencesk[i][j] = 0;
					break;
				case 'T':
					pEvidencesk[i][j] = 1;
					break;
				case 'G':
					pEvidencesk[i][j] = 2;
					break;
				case 'C':
					pEvidencesk[i][j] = 3;
					break;
				default:
					samplelable[i] = 1;	
					
			}
		}
		if (samplelable[i] ==0)
			nEv = nEv + 1;
	}

	CEvidence **pEvidences = new CEvidence *[nEv];
	int dataSize = nnodes;//summ all sizes
	valueVector input_data;
	// input_data.assign(nnodes);
	input_data.resize(nnodes);

#ifdef _OPT
	Value *raw_data;
#ifdef _SIMD_SCORE
	raw_data = (Value *)alignMalloc(sizeof(Value) * (nEv + 4 - (nEv & 3)) * dataSize);
#else
	raw_data = (Value *)alignMalloc(sizeof(Value) * nEv * dataSize);
#endif

	CNodeValues::SetRawDataMatrix(raw_data, dataSize, nEv);
	
	int *obsData;
	// Eric, memory, 4/7/2004
	obsData = (int *) alignMalloc (sizeof(int) * dataSize);
#endif

	int k = -1;
	for (i = 0; i < nEvk; i++)
	{
		if (samplelable[i] == 0)
		{
			k = k + 1;
			for (j = 0; j < 48; j++)
			{
				input_data[j].SetInt((int) pEvidencesk[i][ j +1 ]);
			}
			pEvidences[k] = CEvidence::Create(bNet, nnodes, obs_nodes, input_data);
#ifdef _OPT
			pEvidences[k]->SetEvidenceToMatrix(input_data, k);
			// Eric, memory, 4/7/2004
			pEvidences[k]->SetObsData (obsData);
#endif
		}
	}

	// Eric, memory, 4/7/2004
	for (i = 0; i < nEvk; i++)
	{
		delete pEvidencesk[i];
	}
	delete []pEvidencesk;
	delete []samplelable;
	// End

	intVector vA;
	intVector vD;
	CMlStaticStructLearn *pLearn = CMlStaticStructLearnHC::Create(bNet, itStructLearnML, 
		 StructLearnHC, BIC, 5, vA, vD, 1);

	pLearn -> SetData(nEv, pEvidences);
	pLearn->SetMaxIterIPF(1000);
	static_cast<CMlStaticStructLearnHC*> (pLearn) ->SetMinProgress((float)1e-5);

#ifdef _MPI
	pnlMpiInit(argc, argv);
#endif

        //timer_stop(0);
	//float time1 = timer_read(0);

	printf("Begin Learning ...\n");
        //timer_start(1);

	pLearn ->Learn();

        //timer_stop(1);
	//float time2 = timer_read(1);


#ifdef _MPI
	pnlMpiFinalize();
	// VT_finalize();
#endif

	//////////////////////////////////////////////////////////////////////
	const CDAG* pDAG = pLearn->GetResultDAG();
    //pLearn->CreateResultBNet(const_cast<CDAG *>(pDAG));

	pDAG->Dump();
	delete pLearn;
	for( i = 0; i < nEv; i++)
	{
		delete pEvidences[i];
	}
	delete[] pEvidences;
	
	// Eric, memory, 4/7/2004
	// for( i = 0; i < 620000; i++)
	// {
	// 	delete pEvidencesk[i];
	// }
	// delete[] pEvidencesk;
	// delete[] samplelable;
	// End

	delete obs_nodes;

#ifdef _OPT
	alignFree ((void *)raw_data);
	// alignFree (offset);
	alignFree ((void *)obsData);
#endif

	//printf("Data preparation Time : %.3f\n", time1);
	//printf("Structure learning Time : %.3f\n", time2);

	return 0;
}

