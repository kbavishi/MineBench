// ParseFile.cpp: implementation of the CParseFile class.
//
//////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "string.h"
#include "ParseFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParseFile::CParseFile()
{
	data1 = NULL;
	data2 = NULL;
	subMatrix = NULL;
}

CParseFile::~CParseFile()
{
	ReleaseData();
}

bool CParseFile::LoadFile(int argc, char *argv[])
{
	InitData();
	bool bRet = 1;

	bRet &= GetMatrix(matrix,FT_FULL_SUBMATRIX);
	bRet &= GetData(seq1,FT_PROTEIN | FT_FILE1);
	bRet &= GetData(seq2,FT_PROTEIN | FT_FILE2);
	return bRet;
}

bool CParseFile::GetData(char FileName[], int DataType)
{
	Sequence *data;
	if((DataType & 0x01) == FT_FILE1)
		data = data1;
	else
		data = data2;

	return data->LoadSequenceFromFile(FileName,DataType & 0x02,subMatrix);
}

bool CParseFile::InitData()
{
	ReleaseData();
	data1 = new Sequence;
	data2 = new	Sequence;
	subMatrix = new SubstitutionMatrix;
	return true;
}

void CParseFile::ReleaseData()
{
	Release_Object(data1);
	Release_Object(data2);
	Release_Object(subMatrix);
}

bool CParseFile::GetMatrix(char FileName[], int MatrixType)
{
	return subMatrix->LoadMatrixFromFile(FileName,MatrixType);
}
