// Debug.h: interface for the CDebug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEBUG_H__7DA3D1C7_3F72_4E31_89E1_E529CE1DC4D1__INCLUDED_)
#define AFX_DEBUG_H__7DA3D1C7_3F72_4E31_89E1_E529CE1DC4D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Node;
//class MulArrayNode;
struct Bound;

class ParaSWParam;
class ParaSWGridBlockDivision;
class ParaSWPath;
class ParaSWPathGrid;
class ParaSWProcessorList;

class CDebug  
{
public:
	void PrintMatrixNode2D(Node **m,int row,int col,char* title);
	void PrintMatrixNode2D(Node **m,int row,int col,int offset_col,char* title);
	void PrintMatrixNode1D(Node *m,int row,char* title);
	void PrintPoint(int i,int j,char *title);
	void PrintCheckPoint(int i,int j,ParaSWParam *param,char *title);
	void PrintTitle(char *title1,char *title2);
	void PrintSeparator(char *title1,char *title2);
	void PrintAlignment(char* seq1,char* seq2,char* consensus,int length,char *title);
	void PrintAlignment(ParaSWPath *path,char *title);
	void PrintMatrixInt2D(int **m,int row,int col,char* title);
	void PrintMatrixInt(int* m,int row,int col,char *title);
	void PrintMatrixBound(Bound *m,int row,int col,char *title);
	void PrintParaSWGridBlockDivision(ParaSWGridBlockDivision &pd, char *title);
	void PrintPath(ParaSWPath *path, char *title);
	void PrintPathGrid(ParaSWPathGrid *pg, char *title);
	CDebug();
	virtual ~CDebug();

	void PrintParaSWParam(ParaSWParam *param,char *title);
	void PrintParaSWProcessors(ParaSWProcessorList *pList,char *title);

	FILE *debugFile;
};

#endif // !defined(AFX_DEBUG_H__7DA3D1C7_3F72_4E31_89E1_E529CE1DC4D1__INCLUDED_)
