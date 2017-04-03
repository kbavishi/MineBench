/* SMOOTH.C */
/* This was written by Joachim Stadel and the NASA HPCC ESS at
the University of Washington Department of Astronomy as part of
the SMOOTH program, v2.0.1.
URL: http://www-hpcc.astro.washington.edu/tools/SMOOTH */

/* DJE--I have removed unneeded subroutines, notably those having
to do with velocity field reconstructions (because they refer to
particle data that I chose not to store) and output routines 
(because I wanted binary output).  Also, the density subroutine
was slightly customized to reduce memory consumption in
the case of equal mass particles. */

/* HOP Version 1.0 (12/15/97) -- Original Release */

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <assert.h>
#include "smooth.h"
#include "para_kd.h"

#define IMARK 1		/* All particles are marked to be included */

extern int *start_index;
extern int *local_num_p;
extern int nthreads;
extern float **local_density;

int smInit(SMX *psmx,KD kd,int nSmooth,float *fPeriod)
{
	SMX smx;
	PQ_STATIC;
	int pi,j;
	assert(nSmooth <= kd->nActive);
	smx = (SMX)malloc(sizeof(struct smContext));
	assert(smx != NULL);
	smx->kd = kd;
	smx->nSmooth = nSmooth;
	smx->pfBall2 = (float *)malloc((kd->nActive+1)*sizeof(int));
	assert(smx->pfBall2 != NULL);
        smx->iMark = (char **)calloc(nthreads,sizeof(char *));
        for (j=0; j<nthreads; j++)
          smx->iMark[j]=(char *)calloc(kd->nActive,sizeof(char));
	assert(smx->iMark);
        smx->nListSize = smx->nSmooth+RESMOOTH_SAFE;
         /*
	 ** Set for Periodic Boundary Conditions.
	 */
	for (j=0;j<3;++j) smx->fPeriod[j] = fPeriod[j];
	/*
	 ** Initialize arrays for calculated quantities.--DJE
	 */
	for (pi=0;pi<smx->kd->nActive;++pi) {
		smx->kd->p[pi].fDensity = 0.0;
		smx->kd->p[pi].iHop = 0;
		}
	*psmx = smx;	
	return(1);
	}


void smFinish(SMX smx)
{
    int i;
	free(smx->pfBall2);
        for (i=0; i<nthreads; i++)
          free(smx->iMark[i]);
	free(smx->iMark);
	free(smx);
	}


void smBallSearch(SMX smx,float fBall2,float *ri, PQ **pqH,int pid)
{
	KDN *c;
	PARTICLE *p;
	int cell,cp,ct,pj;
	float temp, fDist2,dx,dy,dz,lx,ly,lz,sx,sy,sz,x,y,z;
	PQ *pq;
	PQ_STATIC;
        c = smx->kd->kdNodes;
	p = smx->kd->p;
	pq = *pqH;
	x = ri[0];
	y = ri[1];
	z = ri[2];
	lx = smx->fPeriod[0];
	ly = smx->fPeriod[1];
	lz = smx->fPeriod[2];
	cell = ROOT;
	/*
	 ** First find the "local" Bucket.
	 ** This could mearly be the closest bucket to ri[3].
	 */
	while (cell < smx->kd->nSplit) {
		if (ri[c[cell].iDim] < c[cell].fSplit) cell = LOWER(cell);
		else cell = UPPER(cell);
		}
	/*
	 ** Now start the search from the bucket given by cell!
	 */
	for (pj=c[cell].pLower;pj<=c[cell].pUpper;++pj) {
		dx = x - p[pj].r[0];
		dy = y - p[pj].r[1];
		dz = z - p[pj].r[2];
		fDist2 = dx*dx + dy*dy + dz*dz;
		if (fDist2 < fBall2) {
			if (smx->iMark[pid][pj]) continue;
			smx->iMark[pid][pq->p] = 0;
			smx->iMark[pid][pj] = 1;
			pq->fKey = fDist2;
			pq->p = pj;
			pq->ax = 0.0;
			pq->ay = 0.0;
			pq->az = 0.0;
			PQ_REPLACE(pq);
			fBall2 = pq->fKey;
			}
		}
	while (cell != ROOT) {
		cp = SIBLING(cell);
		ct = cp;
		SETNEXT(ct);
		while (1) {
			INTERSECT(c,cp,fBall2,lx,ly,lz,x,y,z,sx,sy,sz);
			/*
			 ** We have an intersection to test.
			 */
			if (cp < smx->kd->nSplit) {
				cp = LOWER(cp);
				continue;
				}
			else {
				for (pj=c[cp].pLower;pj<=c[cp].pUpper;++pj) {
					dx = sx - p[pj].r[0];
					dy = sy - p[pj].r[1];
					dz = sz - p[pj].r[2];
					fDist2 = dx*dx + dy*dy + dz*dz;
					if (fDist2 < fBall2) {
						if (smx->iMark[pid][pj]) continue;
					smx->iMark[pid][pq->p] = 0;
						smx->iMark[pid][pj] = 1;
						pq->fKey = fDist2;
						pq->p = pj;
						pq->ax = sx - x;
						pq->ay = sy - y;
						pq->az = sz - z;
						PQ_REPLACE(pq);
						fBall2 = pq->fKey;
						}
					}
				}
		GetNextCell:
			SETNEXT(cp);
			if (cp == ct) break;
			}
		cell = PARENT(cell);
		}
	*pqH = pq;
	}


int smBallGather(SMX smx,float fBall2,float *ri, int *pList,float *fList)
{
	KDN *c;
	PARTICLE *p;
	int pj,nCnt,cp,nSplit;
	float dx,dy,dz,x,y,z,lx,ly,lz,sx,sy,sz,fDist2;

	c = smx->kd->kdNodes;
	p = smx->kd->p;
	nSplit = smx->kd->nSplit;
	lx = smx->fPeriod[0];
	ly = smx->fPeriod[1];
	lz = smx->fPeriod[2];
	x = ri[0];
	y = ri[1];
	z = ri[2];
	nCnt = 0;
	cp = ROOT;
	while (1) {
		INTERSECT(c,cp,fBall2,lx,ly,lz,x,y,z,sx,sy,sz);
		/*
		 ** We have an intersection to test.
		 */
		if (cp < nSplit) {
			cp = LOWER(cp);
			continue;
			}
		else {
			for (pj=c[cp].pLower;pj<=c[cp].pUpper;++pj) {
				dx = sx - p[pj].r[0];
				dy = sy - p[pj].r[1];
				dz = sz - p[pj].r[2];
				fDist2 = dx*dx + dy*dy + dz*dz;
				if (fDist2 < fBall2) {
					fList[nCnt] = fDist2;
					pList[nCnt++] = pj;
					/* Insert debugging flag here */
					if (nCnt > smx->nListSize) {
					    fprintf(stderr,"nCnt too big.\n");
					    }
					}
				}
			}
	GetNextCell:
		SETNEXT(cp);
		if (cp == ROOT) break;
		}
	assert(nCnt <= smx->nListSize);
	return(nCnt);
	}


void smSmooth(SMX smx,void (*fncSmooth)(SMX,int,int,int *,float *,int), int pid)
{
	KDN *c;
        PARTICLE *p;
        PQ *pq,*pqLast, *pqHead,*pqFirst;
	PQ_STATIC;
	int cell;
	int pi,pin,pj,pNext,nCnt,nSmooth;
	float *fList,dx,dy,dz,x,y,z,h2,ax,ay,az;
        int lb, ub, *pList;
        local_density[pid] = (float *)calloc(smx->kd->nActive, sizeof(float));        
        pqFirst = (PQ *)malloc(smx->nSmooth*sizeof(PQ));
        PQ_INIT(pqFirst,smx->nSmooth);
        fList = (float *)malloc(smx->nListSize*sizeof(float));
        pList = (int *)malloc(smx->nListSize*sizeof(int));

        lb=start_index[pid]; 
        ub=lb+local_num_p[pid];     
        for (pi=lb;pi<ub;++pi) {
		if (IMARK) smx->pfBall2[pi] = -1.0;
		else smx->pfBall2[pi] = 1.0;	/* pretend it is already done! */
		}
	smx->pfBall2[smx->kd->nActive] = -1.0; /* stop condition */
	for (pi=0;pi<smx->kd->nActive;++pi) {
		smx->iMark[pid][pi] = 0;
		}
	pqLast = &(pqFirst[smx->nSmooth-1]);
	c = smx->kd->kdNodes;
	p = smx->kd->p;
	nSmooth = smx->nSmooth;
	/*
	 ** Initialize Priority Queue.
	 */
	pin = lb;
	pNext = lb+1;
	ax = 0.0;
	ay = 0.0;
	az = 0.0;
	for (pq=pqFirst,pj=lb;pq<=pqLast;++pq,++pj) {
		smx->iMark[pid][pj] = 1;
		pq->p = pj;
		pq->ax = ax;
		pq->ay = ay;
		pq->az = az;
		}
	while (1) {
		if (smx->pfBall2[pin] >= 0) {
			/*
			 ** Find next particle which is not done, and load the
			 ** priority queue with nSmooth number of particles.
			 */
			while (smx->pfBall2[pNext] >= 0) ++pNext;
			/*
			 ** Check if we are really finished.
			 */
			if (pNext >=ub) break;
			pi = pNext;
			++pNext;
			x = p[pi].r[0];
			y = p[pi].r[1];
			z = p[pi].r[2];
			/* printf("%d: %g %g %g\n", pi, x, y, z); */
			/*
			 ** First find the "local" Bucket.
			 ** This could mearly be the closest bucket to ri[3].
			 */
			cell = ROOT;
			while (cell < smx->kd->nSplit) {
				if (p[pi].r[c[cell].iDim] < c[cell].fSplit)
					cell = LOWER(cell);
				else
					cell = UPPER(cell);
				}
			/*
			 ** Remove everything from the queue.
			 */
			pqHead = NULL;
			for (pq=pqFirst;pq<=pqLast;++pq) smx->iMark[pid][pq->p] = 0;
			/*
			 ** Add everything from pj up to and including pj+nSmooth-1.
			 */
			pj = c[cell].pLower;
			if (pj > ub - nSmooth)
				pj = ub - nSmooth;
			for (pq=pqFirst;pq<=pqLast;++pq) {
				smx->iMark[pid][pj] = 1;
				dx = x - p[pj].r[0];
				dy = y - p[pj].r[1];
				dz = z - p[pj].r[2];
				pq->fKey = dx*dx + dy*dy + dz*dz;
				pq->p = pj++;
				pq->ax = 0.0;
				pq->ay = 0.0;
				pq->az = 0.0;
				}
			PQ_BUILD(pqFirst,nSmooth,pqHead);
			}
		else {
			/*
			 ** Calculate the priority queue using the previous particles!
			 */
			pi = pin;
			x = p[pi].r[0];
			y = p[pi].r[1];
			z = p[pi].r[2];
			pqHead = NULL;
			for (pq=pqFirst;pq<=pqLast;++pq) {
				pq->ax -= ax;
				pq->ay -= ay;
				pq->az -= az;
				dx = x + pq->ax - p[pq->p].r[0];
				dy = y + pq->ay - p[pq->p].r[1];
				dz = z + pq->az - p[pq->p].r[2];
				pq->fKey = dx*dx + dy*dy + dz*dz;
				}
			PQ_BUILD(pqFirst,nSmooth,pqHead);
			ax = 0.0;
			ay = 0.0;
			az = 0.0;
			}
                smBallSearch(smx,pqHead->fKey,p[pi].r, &pqHead,pid);
		smx->pfBall2[pi] = pqHead->fKey;
		/*
		 ** Pick next particle, 'pin'.
		 ** Create fList and pList for function 'fncSmooth'.
		 */
		pin = pi;
		nCnt = 0;
		h2 = pqHead->fKey;
		for (pq=pqFirst;pq<=pqLast;++pq) {
			if (pq == pqHead) continue;
			pList[nCnt] = pq->p;
			fList[nCnt++] = pq->fKey;
			if (smx->pfBall2[pq->p] >= 0) continue;
			if ((pq->fKey < h2)&&(pq->p < ub)&&(pq->p>=lb)) {
				pin = pq->p;
				h2 = pq->fKey;
				ax = pq->ax;
				ay = pq->ay;
				az = pq->az;
				}
			}
		(*fncSmooth)(smx,pi,nCnt,pList,fList,pid);
                }
	}


void smReSmooth(SMX smx,void (*fncSmooth)(SMX,int,int,int *,float *, int),int pid)
{
	PARTICLE *p;
	int lb, ub,pi,nSmooth;
        float *fList;
        int *pList;
        fList = (float *)malloc(smx->nListSize*sizeof(float));
        pList = (int *)malloc(smx->nListSize*sizeof(int));
    
        lb=start_index[pid];
        ub=start_index[pid] + local_num_p[pid]; 
	p = smx->kd->p;
	for (pi=lb;pi<ub;++pi) {
		if (IMARK == 0) continue;
		/*
		 ** Do a Ball Gather at the radius of the most distant particle
		 ** which is smDensity sets in smx->pBall[pi].
		 */
		nSmooth = smBallGather(smx,smx->pfBall2[pi],p[pi].r, pList, fList);
		(*fncSmooth)(smx,pi,nSmooth,pList,fList, pid);
		}
 	}


void smDensity(SMX smx,int pi,int nSmooth,int *pList,float *fList, int pid)
{
	float ih2,r2,rs,fDensity;
	int i,pj;

	ih2 = 4.0/smx->pfBall2[pi];
	fDensity = 0.0;
	for (i=0;i<nSmooth;++i) {
		pj = pList[i];
		r2 = fList[i]*ih2;
		rs = 2.0 - sqrt(r2);
		if (r2 < 1.0) rs = (1.0 - 0.75*rs*r2);
		else rs = 0.25*rs*rs*rs;
#ifdef DIFFERENT_MASSES
		fDensity += rs*smx->kd->p[pj].fMass;
#else
		fDensity += rs*smx->kd->fMass;
#endif
		}
	smx->kd->p[pi].fDensity = M_1_PI*sqrt(ih2)*ih2*fDensity; 
	}


void smDensitySym(SMX smx,int pi,int nSmooth,int *pList,float *fList, int pid)
{
	float fNorm,ih2,r2,rs;
	int i,pj;
        ih2 = 4.0/smx->pfBall2[pi];
	fNorm = 0.5*M_1_PI*sqrt(ih2)*ih2;
	for (i=0;i<nSmooth;++i) {
		pj = pList[i];
		r2 = fList[i]*ih2;
		rs = 2.0 - sqrt(r2);
                if (r2 < 1.0) rs = (1.0 - 0.75*rs*r2);
		else rs = 0.25*rs*rs*rs;
		rs *= fNorm;
#ifdef DIFFERENT_MASSES
                local_density[pid][pi] += rs*smx->kd->p[pj].fMass;
                local_density[pid][pj] += rs*smx->kd->p[pi].fMass;
#else
                smx->kd->p[pi].fDensity += rs*smx->kd->fMass;
                smx->kd->p[pj].fDensity += rs*smx->kd->fMass;
#endif
		}
	}

/* I'm not using the following function, but I left it here in case someone
wants the densities outputted in Tipsy format.  But you're probably better
off just fetching the smooth() program from the HPCC web site... */

void smOutDensity(SMX smx,FILE *fp)
{
	int i,iCnt;

	fprintf(fp,"%d\n",smx->kd->nParticles);
	iCnt = 0;
	for (i=0;i<smx->kd->nGas;++i) {
		if (smx->kd->bGas) {
			if (IMARK)
				fprintf(fp,"%.8g\n",smx->kd->p[iCnt].fDensity);
			else fprintf(fp,"0\n");
			++iCnt;
			}
		else fprintf(fp,"0\n");
		}
	for (i=0;i<smx->kd->nDark;++i) {
		if (smx->kd->bDark) {
			if (IMARK)
				fprintf(fp,"%.8g\n",smx->kd->p[iCnt].fDensity);
			else fprintf(fp,"0\n");
			++iCnt;
			}
		else fprintf(fp,"0\n");
		}
	for (i=0;i<smx->kd->nStar;++i) {
		if (smx->kd->bStar) {
			if (IMARK)
				fprintf(fp,"%.8g\n",smx->kd->p[iCnt].fDensity);
			else fprintf(fp,"0\n");
			++iCnt;
			}
		else fprintf(fp,"0\n");
		}
	}
