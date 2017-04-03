// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdlib.h>

#include<omp.h>
#include<string.h>
// TODO: reference additional headers your program requires here
//definition for release memory:
#define ALIGN_BYTES		128

char *alignMalloc( int len  );
void alignFree(void * p);

#if 1
#define Release_Mem(p,len) if((p)!=NULL) delete[] (p); (p) = NULL; (len) = 0;
#define Release_Mem_2(p,len) if((p)!=NULL) delete[] (p); (p) = NULL;
#define Release_Object(p) if((p)!=NULL) delete (p); (p) = NULL;
#define Release_Mem2D(p,dim1,dim2) if((p)!=NULL) {							\
						if((p)[0]!=NULL) delete [] (p)[0];		\
						delete [] (p); (p) = NULL;	};
#define Release_Mem2D_0(p,dim1,dim2) if((p)!=NULL) {						\
						if((p)[0]!=NULL) delete [] (p)[0];		\
						delete [] (p); (p) = NULL;					\
						(dim1) = 0; (dim2) = 0;	};
/* #define Release_Mem2D_1(p,dim1,dim2,px) if(px!=NULL) {		\
						delete [] px;		\
						delete [] p;					\
						p = NULL; px = NULL;				\
						} */
#define Release_Mem2D_1(p,dim1,dim2,px) if(px!=NULL) {		\
						alignFree (px);		\
						delete [] p;					\
						p = NULL; px = NULL;				\
						}
#define Release_Mem_1(p,len,px) if((px)!=NULL) delete[] (px); (p) = NULL; (px) = NULL;

//definition for allocate memory:
#define Allocate_Mem(p,t,len) (p) = new t[(len)];
#define Allocate_Mem2D(p,t,dim1,dim2) (p) = new t*[(dim1)];		\
						(p)[0] = new t[(dim1)*(dim2)];			\
						{										\
							for(int _i=0;_i<(dim1);_i++)			\
								(p)[_i] = &((p)[0][(dim2)*_i]);	\
						};
/*#define Allocate_Mem2D_1(p,t,dim1,dim2,px) (p) = new t*[(dim1)];	\
						px = new t[(dim1)*(dim2)+1];				\
						{											\
							for(int _i=0;_i<(dim1);_i++)			\
								(p)[_i] = &px[(dim2)*_i+1];			\
						} */
#define Allocate_Mem2D_1(p,t,dim1,dim2,px) (p) = new t*[(dim1)];	\
						px = (t *)alignMalloc( sizeof(t) * ((dim1)*(dim2)+1));\
						{											\
							for(int _i=0;_i<(dim1);_i++)			\
								(p)[_i] = &px[(dim2)*_i+1];			\
						}
#define Allocate_Mem_1(p,t,len,px) px = new t[(len)+1]; p = px+1;

#else	// aligned malloc & free

#define Release_Mem(p,len) if((p)!=NULL) delete[] (p); (p) = NULL; (len) = 0;
#define Release_Mem_2(p,len) if((p)!=NULL) delete[] (p); (p) = NULL;
#define Release_Object(p) if((p)!=NULL) delete (p); (p) = NULL;
// #define Release_Mem(p,len) if((p)!=NULL) alignFree (p); (p) = NULL; (len) = 0;
// #define Release_Mem_2(p,len) if((p)!=NULL) alignFree (p); (p) = NULL;
// #define Release_Object(p) if((p)!=NULL) alignFree (p); (p) = NULL;
#define Release_Mem2D(p,dim1,dim2) if((p)!=NULL) {							\
						if((p)[0]!=NULL) delete [] (p)[0];		\
						delete [] (p); (p) = NULL;	};
#define Release_Mem2D_0(p,dim1,dim2) if((p)!=NULL) {						\
						if((p)[0]!=NULL) alignFree ((p)[0]);		\
						delete []p; (p) = NULL;					\
						(dim1) = 0; (dim2) = 0;	};
#define Release_Mem2D_1(p,dim1,dim2,px) if(px!=NULL) {		\
						alignFree (px);		\
						delete [] p;					\
						p = NULL; px = NULL;				\
						}
#define Release_Mem_1(p,len,px) if((px)!=NULL) alignFree (px); (p) = NULL; (px) = NULL;

//definition for allocate memory:
#define Allocate_Mem(p,t,len) (p) = new t[(len)];
// #define Allocate_Mem(p,t,len) (p) = (t *)alignMalloc(sizeof(t) * (len));
#define Allocate_Mem2D(p,t,dim1,dim2) (p) = new t*[(dim1)];		\
						(p)[0] = (t *)alignMalloc( sizeof(t) * (dim1)*(dim2));\
						{										\
							for(int _i=0;_i<(dim1);_i++)			\
								(p)[_i] = &((p)[0][(dim2)*_i]);	\
						};
#define Allocate_Mem2D_1(p,t,dim1,dim2,px) (p) = new t*[(dim1)];	\
						px = (t *)alignMalloc( sizeof(t) * ((dim1)*(dim2)+1));\
						{											\
							for(int _i=0;_i<(dim1);_i++)			\
								(p)[_i] = &px[(dim2)*_i+1];			\
						}
#define Allocate_Mem_1(p,t,len,px) px = (t *)alignMalloc(sizeof(t) * ((len)+1)); p = px+1;

#endif

#endif // !defined(_COMMON_H)
