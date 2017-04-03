// ParseFile.h: interface for the CParseFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARSEFILE_H__7E73DF21_AC74_48FE_B608_3F5E33FA8760__INCLUDED_)
#define AFX_PARSEFILE_H__7E73DF21_AC74_48FE_B608_3F5E33FA8760__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Debug.h"
#include "ParaSWParam.h"

extern CDebug debugf;

class CParseFile  
{
public:
	bool GetMatrix(char FileName[],int MatrixType);
	void ReleaseData();
	bool InitData();
	bool GetData(char FileName[],int DataType);
	bool LoadFile(int argc, char *argv[]);
	CParseFile();
	virtual ~CParseFile();

	ParaSWParam * param;	//param used in Parallel SW

	Sequence *data1;
	Sequence *data2;

	SubstitutionMatrix *subMatrix;

	// Add by Eric
	char seq1[100];
	char seq2[100];
	char matrix[100];

};

#endif // !defined(AFX_PARSEFILE_H__7E73DF21_AC74_48FE_B608_3F5E33FA8760__INCLUDED_)
