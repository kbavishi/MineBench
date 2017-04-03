// Debug.cpp: implementation of the CDebug class.
//
//////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "Debug.h"
#include "ParaSWParam.h"
#include "ParaSWProblem.h"
#include "ParaSWPath.h"
#include "ParaSWProcessorSet.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

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

CDebug::CDebug()
{
	debugFile = fopen("debug1.txt","w");
}

CDebug::~CDebug()
{
	fclose(debugFile);
}

void CDebug::PrintMatrixInt(int *m, int row, int col,char *title)
{
	int i,j;
	PrintTitle("PrintMatrixInt:",title);
	for(i=0;i<row;i++)
	{
		for(j=0;j<col;j++)
		{
			fprintf(debugFile,"%d\t",m[i*col+j]);
		}
		fprintf(debugFile,"\n");
	}
}

void CDebug::PrintMatrixInt2D(int **m, int row, int col, char *title)
{
	int i,j;
	PrintTitle("PrintMatrixInt2D:",title);
	for(i=0;i<row;i++)
	{
		for(j=0;j<col;j++)
		{
			fprintf(debugFile,"%d\t",m[i][j]);
		}
		fprintf(debugFile,"\n");
	}
}

void CDebug::PrintAlignment(ParaSWPath *path,char *title)
{
	PrintTitle("PrintAlignment:",title);
	int i,j;
	fprintf(debugFile,"seq1: start = %d; end = %d\n",path->seq1_start,path->seq1_end);
	fprintf(debugFile,"seq2: start = %d; end = %d\n",path->seq2_start,path->seq2_end);
	for(i=0;i<path->length;i+=100)
	{
		for(j=0;j<100 && j<path->length-i;j++)	fprintf(debugFile,"%c",path->seq1[i+j]);
		fprintf(debugFile,"\n");
		for(j=0;j<100 && j<path->length-i;j++)	fprintf(debugFile,"%c",path->ali[i+j]);
		fprintf(debugFile,"\n");
		for(j=0;j<100 && j<path->length-i;j++)	fprintf(debugFile,"%c",path->seq2[i+j]);
		fprintf(debugFile,"\n");
		fprintf(debugFile,"\n");
	}
//	fprintf(debugFile,"%s\n",seq1);
//	if(consensus!=NULL) fprintf(debugFile,"%s\n",consensus);
// 	fprintf(debugFile,"%s\n",seq2);
}

void CDebug::PrintAlignment(char* seq1,char* seq2,char* consensus,int length,char *title)
{
	PrintTitle("PrintAlignment:",title);
	int i,j;
	for(i=0;i<length;i+=100)
	{
		for(j=0;j<100 && j<length-i;j++)	fprintf(debugFile,"%c",seq1[i+j]);
		fprintf(debugFile,"\n");
		for(j=0;j<100 && j<length-i;j++)	fprintf(debugFile,"%c",consensus[i+j]);
		fprintf(debugFile,"\n");
		for(j=0;j<100 && j<length-i;j++)	fprintf(debugFile,"%c",seq2[i+j]);
		fprintf(debugFile,"\n");
		fprintf(debugFile,"\n");
	}
//	fprintf(debugFile,"%s\n",seq1);
//	if(consensus!=NULL) fprintf(debugFile,"%s\n",consensus);
// 	fprintf(debugFile,"%s\n",seq2);
}

void CDebug::PrintTitle(char *title1,char *title2)
{
	fprintf(debugFile,"--------------------------------------------------\n");
	fprintf(debugFile,"%s\n",title1);
	fprintf(debugFile,"%s\n",title2);
}

void CDebug::PrintSeparator(char *title1,char *title2)
{
	fprintf(debugFile,"\n================================================================================\n");
	fprintf(debugFile,"%s\n",title1);
	fprintf(debugFile,"%s\n",title2);
}

void CDebug::PrintPoint(int i, int j, char *title)
{
	PrintTitle("PrintPoint:",title);
	fprintf(debugFile,"(%d,%d)\n",i,j);
}

void CDebug::PrintCheckPoint(int i,int j,ParaSWParam *param,char *title)
{
	if(i<0 || j<0 || 
		i>=param->grid_height_count || 
		i>=param->grid_width_count || 
		j>=param->seq1->length ||
		j>=param->seq2->length)
	{
		PrintTitle("PrintCheckPoint:",title);
		fprintf(debugFile,"(%d,%d)\n",i,j);
	}
}

void CDebug::PrintParaSWGridBlockDivision(ParaSWGridBlockDivision &pd, char *title)
{
	PrintSeparator("PrintParaSWGridBlockDivision:",title);
//	PrintTitle("PrintParaSWGridBlockDivision:",title);
	char c[500];
	sprintf(c,"pd.blockHeights_count = %d\npd.gridHeights_count = %d",pd.blockHeights_count,pd.gridHeights_count);
	PrintTitle("",c);
	PrintMatrixBound(pd.blockHeights,pd.blockHeights_count,1,"pd.blockHeights");
	PrintMatrixBound(pd.gridHeights,pd.gridHeights_count,1,"pd.gridHeights");
	PrintMatrixInt(pd.blockHeightOnGrid,pd.blockHeights_count,1,"pd.blockHeightOnGrid");

	sprintf(c,"pd.blockWidths_count = %d\npd.gridWidths_count = %d",pd.blockWidths_count,pd.gridWidths_count);
	PrintTitle("",c);
	PrintMatrixBound(pd.blockWidths,pd.blockWidths_count,1,"pd.blockWidths");
	PrintMatrixBound(pd.gridWidths,pd.gridWidths_count,1,"pd.gridWidths");
	PrintMatrixInt(pd.blockWidthOnGrid,pd.blockWidths_count,1,"pd.blockWidthOnGrid");
}

void CDebug::PrintMatrixBound(Bound *m,int row,int col,char *title)
{
	int i,j;
	PrintTitle("PrintMatrixInt:",title);
	for(i=0;i<row;i++)
	{
		for(j=0;j<col;j++)
		{
			fprintf(debugFile,"(%d,%d)\t",m[i*col+j].start,m[i*col+j].end);
		}
		fprintf(debugFile,"\n");
	}
}

void CDebug::PrintPathGrid(ParaSWPathGrid *pg, char *title)
{
	FILE *f = debugFile;
	PrintTitle("PrintPathGrid:",title);
	fprintf(f,"start:(%d,%d)(%d)----->end:(%d,%d)(%d); delta score = %d\n",
				pg->start.i,pg->start.j,pg->end_node.trace_start[pg->trace_num],
				pg->end.i,pg->end.j,pg->trace_num,pg->delta_score);
}

void CDebug::PrintParaSWProcessors(ParaSWProcessorList *pList,char *title)
{
	ParaSWProcessorListNode *pn;
	pn = pList->first;
	PrintSeparator("PrintParaSWProcessors:",title);
//	PrintTitle("PrintParaSWProcessors:",title);
	while(pn!=NULL)
	{
		fprintf(debugFile,"--------------------------------------------------\n");
		fprintf(debugFile,"processorCount = %d\nprocessorID = %d\nblockMaxHeight = %d\nblockMaxWidth = %d\n",
			pn->p_proc->processorCount,
			pn->p_proc->processorID,
			pn->p_proc->blockMaxHeight,
			pn->p_proc->blockMaxWidth);
		pn = pn->next;
	}

}

void CDebug::PrintPath(ParaSWPath *path,char *title)
{
	ParaSWPathGrid *pg;
	FILE *f = debugFile;
	PrintSeparator("PrintPath:",title);
//	PrintTitle("PrintPath:",title);
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"Number of UnSolved path grids: %d\n",path->unSolvedPathGrids.GetCount());
//	fprintf(f,"Number of Solvable path grids: %d\n",path->solvablePathGrids.GetCount());
	fprintf(f,"Number of Solved path grids: %d\n",path->solvedPathGrids.GetCount());
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"UnSolved path grids:\n");
	pg = path->unSolvedPathGrids.first;
	int score = 0;
	while(pg!=NULL)
	{
		fprintf(f,"start:(%d,%d)(%d)----->end:(%d,%d)(%d); delta score = %d\n",
				pg->start.i,pg->start.j,pg->end_node.trace_start[pg->trace_num],
				pg->end.i,pg->end.j,pg->trace_num,pg->delta_score);
		score += pg->delta_score;
		pg = pg->next;
	}
//	fprintf(f,"--------------------------------------------------\n");
//	fprintf(f,"Solvable path grids:\n");
//	pg = path->solvablePathGrids.first;
//	while(pg!=NULL)
//	{
//		fprintf(f,"start:(%d,%d)(%d)----->end:(%d,%d)(%d); delta score = %d\n",
//				pg->start.i,pg->start.j,pg->end_node.trace_start[pg->trace_num],
//				pg->end.i,pg->end.j,pg->trace_num,pg->delta_score);
//		score += pg->delta_score;
//		pg = pg->next;
//	}
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"Solved path grids:\n");
	pg = path->solvedPathGrids.first;
	while(pg!=NULL)
	{
		fprintf(f,"start:(%d,%d)(%d)----->end:(%d,%d)(%d); delta score = %d\n",
				pg->start.i,pg->start.j,pg->end_node.trace_start[pg->trace_num],
				pg->end.i,pg->end.j,pg->trace_num,pg->delta_score);
		score += pg->delta_score;
		pg = pg->next;
	}
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"total score = %d\n",score);
}

void CDebug::PrintParaSWParam(ParaSWParam *param,char *title)
{
	int i,j;
	FILE *f = debugFile;
	PrintSeparator("PrintParaSWParam:",title);
//	PrintTitle("PrintParaSWParam:",title);
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"align_type = %d\n",param->align_type);
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"seq1: type = %d, length = %d\ndata: ",param->seq1->type,param->seq1->length);
	for(i=0;i<param->seq1->length;i++)
	{
		fprintf(f,"%c",param->seq1->data[i]);
	}
	fprintf(f,"\n");
	for(i=0;i<param->seq1->length;i++)
	{
		fprintf(f,"%d\t",param->seq1->index[i]);
	}
	fprintf(f,"\n");
	fprintf(f,"seq2: type = %d, length = %d\ndata: ",param->seq2->type,param->seq2->length);
	for(i=0;i<param->seq2->length;i++)
	{
		fprintf(f,"%c",param->seq2->data[i]);
	}
	fprintf(f,"\n");
	for(i=0;i<param->seq2->length;i++)
	{
		fprintf(f,"%d\t",param->seq2->index[i]);
	}
	fprintf(f,"\n");
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"substitution matrix pssm: %d\n",param->pssm->name);
	for(i=0;i<param->pssm->dimension;i++)
	{
		for(j=0;j<param->pssm->dimension;j++)
		{
			fprintf(f,"%d\t",param->pssm->score[i][j]);
		}
		fprintf(f,"\n");
	}
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"gap open = %d\n",param->gap_open);
	fprintf(f,"gap extend = %d\n",param->gap_extend);
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"processor count = %d\n",param->processor_count);
	fprintf(f,"block width = %d\n",param->block_width);
	for(i=0;i<param->processor_count;i++)
	{
		fprintf(f,"block_height[%d] = %d\n",i,param->block_height[i]);
	}
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"grid height count = %d\n",param->grid_height_count);
	fprintf(f,"grid width count = %d\n",param->grid_width_count);
	fprintf(f,"--------------------------------------------------\n");
	fprintf(f,"k_path = %d\n",param->k);
}


void CDebug::PrintMatrixNode1D(Node *m,int row,char* title)
{
	int i,n;
	PrintTitle("PrintMatrixNode1D:",title);
	fprintf(debugFile,"score:\n");
	for(i=0;i<row;i++)
	{
		fprintf(debugFile,"(");
		for(n=0;n<Node_length;n++)
		{
			fprintf(debugFile,"|%d|",m[i].score[n]);
		}
		fprintf(debugFile,")\t");
	}
	fprintf(debugFile,"\n");
	fprintf(debugFile,"trace:\n");
	for(i=0;i<row;i++)
	{
		fprintf(debugFile,"(");
		for(n=0;n<Node_length;n++)
		{
			fprintf(debugFile,"|%d|",m[i].trace[n]);
		}
		fprintf(debugFile,")\t");
	}
	fprintf(debugFile,"\n");
	fprintf(debugFile,"global_start:\n");
	for(i=0;i<row;i++)
	{
		fprintf(debugFile,"(");
		for(n=0;n<Node_length;n++)
		{
			fprintf(debugFile,"|%d,%d|",m[i].global_start[n].i,m[i].global_start[n].j);
		}
		fprintf(debugFile,")\t");
	}
	fprintf(debugFile,"\n");
	fprintf(debugFile,"local_start:\n");
	for(i=0;i<row;i++)
	{
		fprintf(debugFile,"(");
		for(n=0;n<Node_length;n++)
		{
			fprintf(debugFile,"|%d,%d|",m[i].local_start[n].i,m[i].local_start[n].j);
		}
		fprintf(debugFile,")\t");
	}
	fprintf(debugFile,"\n");
	fprintf(debugFile,"trace_start:\n");
	for(i=0;i<row;i++)
	{
		fprintf(debugFile,"(");
		for(n=0;n<Node_length;n++)
		{
			fprintf(debugFile,"|%d|",m[i].trace_start[n]);
		}
		fprintf(debugFile,")\t");
	}
	fprintf(debugFile,"\n");
}

void CDebug::PrintMatrixNode2D(Node **m, int row, int col,int offset_col, char *title)
{
	int i,j;
	int n;
	PrintSeparator("PrintMatrixNode2D:",title);
	PrintTitle("PrintMatrixNode2D:",title);
	fprintf(debugFile,"score:\n");
	for(i=0;i<row;i++)
	{
		for(j=offset_col;j<col+offset_col;j++)
		{
			fprintf(debugFile,"(%d,%d)(",i,j);
			for(n=0;n<Node_length;n++)
			{
				fprintf(debugFile,"|%d|",m[i][j].score[n]);
			}
			fprintf(debugFile,")\t");
		}
		fprintf(debugFile,"\n");
	}
	fprintf(debugFile,"\n");
	fprintf(debugFile,"trace:\n");
	for(i=0;i<row;i++)
	{
		for(j=offset_col;j<col+offset_col;j++)
		{
			fprintf(debugFile,"(%d,%d)(",i,j);
			for(n=0;n<Node_length;n++)
			{
				fprintf(debugFile,"|%d|",m[i][j].trace[n]);
			}
			fprintf(debugFile,")\t");
		}
		fprintf(debugFile,"\n");
	}
	fprintf(debugFile,"\n");
	fprintf(debugFile,"global_start:\n");
	for(i=0;i<row;i++)
	{
		for(j=offset_col;j<col+offset_col;j++)
		{
			fprintf(debugFile,"(%d,%d)(",i,j);
			for(n=0;n<Node_length;n++)
			{
				fprintf(debugFile,"|%d,%d|",m[i][j].global_start[n].i,m[i][j].global_start[n].j);
			}
			fprintf(debugFile,")\t");
		}
		fprintf(debugFile,"\n");
	}
	fprintf(debugFile,"\n");
	fprintf(debugFile,"local_start,trace_start:\n");
	for(i=0;i<row;i++)
	{
		for(j=offset_col;j<col+offset_col;j++)
		{
			fprintf(debugFile,"(%d,%d)(",i,j);
			for(n=0;n<Node_length;n++)
			{
				fprintf(debugFile,"|%d,%d,(%d)|",m[i][j].local_start[n].i,m[i][j].local_start[n].j,m[i][j].trace_start[n]);
			}
			fprintf(debugFile,")\t");
		}
		fprintf(debugFile,"\n");
	}
	fprintf(debugFile,"\n");
}

void CDebug::PrintMatrixNode2D(Node **m, int row, int col, char *title)
{
	int i,j;
	int n;
	PrintSeparator("PrintMatrixNode2D:",title);
//	PrintTitle("PrintMatrixNode2D:",title);
	fprintf(debugFile,"score:\n");
	for(i=0;i<row;i++)
	{
		for(j=0;j<col;j++)
		{
			fprintf(debugFile,"(%d,%d)(",i,j);
			for(n=0;n<Node_length;n++)
			{
				fprintf(debugFile,"|%d|",m[i][j].score[n]);
			}
			fprintf(debugFile,")\t");
		}
		fprintf(debugFile,"\n");
	}
	fprintf(debugFile,"\n");
	fprintf(debugFile,"trace:\n");
	for(i=0;i<row;i++)
	{
		for(j=0;j<col;j++)
		{
			fprintf(debugFile,"(%d,%d)(",i,j);
			for(n=0;n<Node_length;n++)
			{
				fprintf(debugFile,"|%d|",m[i][j].trace[n]);
			}
			fprintf(debugFile,")\t");
		}
		fprintf(debugFile,"\n");
	}
	fprintf(debugFile,"\n");
	fprintf(debugFile,"global_start:\n");
	for(i=0;i<row;i++)
	{
		for(j=0;j<col;j++)
		{
			fprintf(debugFile,"(%d,%d)(",i,j);
			for(n=0;n<Node_length;n++)
			{
				fprintf(debugFile,"|%d,%d|",m[i][j].global_start[n].i,m[i][j].global_start[n].j);
			}
			fprintf(debugFile,")\t");
		}
		fprintf(debugFile,"\n");
	}
	fprintf(debugFile,"\n");
	fprintf(debugFile,"local_start,trace_start:\n");
	for(i=0;i<row;i++)
	{
		for(j=0;j<col;j++)
		{
			fprintf(debugFile,"(%d,%d)(",i,j);
			for(n=0;n<Node_length;n++)
			{
				fprintf(debugFile,"|%d,%d,(%d)|",m[i][j].local_start[n].i,m[i][j].local_start[n].j,m[i][j].trace_start[n]);
			}
			fprintf(debugFile,")\t");
		}
		fprintf(debugFile,"\n");
	}
	fprintf(debugFile,"\n");
}

