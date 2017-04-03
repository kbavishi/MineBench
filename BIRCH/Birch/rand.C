/****************************************************************************

     Copyright (c) 1993 1994 1995
     By Miron Livny, Madison, Wisconsin
     All Rights Reserved.

     UNDER NO CIRCUMSTANCES IS THIS PROGRAM TO BE COPIED OR DISTRIBUTED
     WITHOUT PERMISSION OF MIRON LIVNY

     obtained and modified on 3/15/96 by Tian Zhang
****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <malloc.h>

#include "rand.h"

#define  DRAW	s1 = s1* 1103515245 + 12345
#define	 ABS(x)	(x & 0x7fffffff)
#define	 MASK(x) ABS(x)
#define  MAX_INT 2147483648

int	start = 1;
void	init();
int	binary_search(int, int, float*, float);

enum SeedType  { InitialSeed, LastSeed, NewSeed } ;
typedef int *IntArrayPtr;
typedef char *BooleanArrayPtr;

#define M1   2147483563
#define M2   2147483399
#define A1   40014
#define A2   40692
#define w    30 
#define A1W   1033780774
#define A2W   1494757890     
#define MAXG  1073741824    
#define UnSurM1   4.656613057e-10

IntArrayPtr Ig1, Ig2, Lg1, Lg2, Cg1, Cg2 ;
IntArrayPtr Antithetic ; 
int   A1VW, A2VW,  NumberOfGenerators;
float   ln10;

#define   H  32768
/*=================================================================*/
int MultMod( int a, int s, int m ) {
int   a0;
int   a1, q, qh, rh, k, p ;
   if ( a < H ) {
      a0 = a;  p = 0;
    }
   else {
      a1 = a / H;   a0 = a - H * a1;
      qh = m / H;   rh = m - H * qh;
      if ( a1 >= H ) { 
         a1 = a1 - H;  k = s / qh;
         p = H * (s - k * qh) - k * rh;
         while ( p < 0 ) {  p = p + m;  }
         }
	else {
         p = 0;
      };
      if ( a1 != 0 ) {
         q = m / a1;  k = s / q;
         p = p - k * (m - a1 * q);
         if ( p > 0 ) {  p = p - m ; };
         p = p + a1 * (s - k * q);
         while ( p < 0 ) {  p = p + m;  };
      };          
      k = p / qh;
      p = H * (p - k * qh) - k * rh;
      while ( p < 0 ) {  p = p + m;  };
   };            
   if ( a0 != 0 ) {
      q = m / a0;  k = s / q;
      p = p - k * (m - a0 * q);
      if ( p > 0 ) {  p = p - m;  };
      p = p + a0 * (s - k * q);
      while ( p < 0 ) {  p = p + m;  };
   };
   return p;
};

/*=================================================================*/
void GetState( int g , int  *S1, int *S2 ) {
   *S1 = Cg1[g];  *S2 = Cg2[g];
};

/*=================================================================*/
void AdvanceState( int g , int k ){
    int B1, B2, N ;
    int i ;
    B1 = A1;  B2 = A2;
    for ( i = 1; i <= k-1 ; i++) {
        B1 = MultMod( B1, A1, M1 );
        B2 = MultMod( B2, A2, M2 );
    };
   /* B1 = A1 ** k   and   B2 = A2 ** k.    */
   Cg1[g] = MultMod( B1, Cg1[g], M1 ) ;
   Cg2[g] = MultMod( B2, Cg2[g], M2 ) ;
   N = MultMod( A1W, Lg1[g], M1 ) ;
   while ( Cg1[g] >= N ) {
	  Lg1[g] = N ;
	  N = MultMod( A1W, Lg1[g], M1 ) ;
   };
   N = MultMod( A2W, Lg2[g], M2 ) ;
   while ( Cg2[g] >= N ) {
	  Lg2[g] = N ;
	  N = MultMod( A2W, Lg2[g], M2 );
       } ;
    };

/*=================================================================*/
void InitGenerator( int g , SeedType Where ){
    switch (Where) {
       case InitialSeed  :  
          Lg1[g] = Ig1[g];
          Lg2[g] = Ig2[g];
	  break;
       case NewSeed :
           Lg1[g] = MultMod( A1W, Lg1[g], M1 );
           Lg2[g] = MultMod( A2W, Lg2[g], M2 );
           break;
       case LastSeed :
           break;
       };
   Cg1[g] = Lg1[g];  
   Cg2[g] = Lg2[g];
   };

/*=================================================================*/
void SetSeed( int g , int S1, int S2 ) {
   Ig1[g] = S1;  Ig2[g] = S2;
   InitGenerator( g, InitialSeed );
};

/*=================================================================*/
void ResetGenerators( SeedType Where ){
    for (int g = 1;g <= NumberOfGenerators; g++) InitGenerator(g,Where);
};

/*=================================================================*/
void SetInitialSeed( int S1, int S2 ){
   Ig1[1] = S1;  Ig2[1] = S2;
   InitGenerator( 1, InitialSeed );
    for ( int g = 2 ; g <= NumberOfGenerators; g++ ) {
      Ig1[g] = MultMod( A1VW, Ig1[g-1], M1 );
      Ig2[g] = MultMod( A2VW, Ig2[g-1], M2 );
      InitGenerator( g, InitialSeed );
   }
}; 

/*=================================================================*/
void SetAntithetic ( int g, int B ) {
   Antithetic[g] = B;
};

/*=================================================================*/
int Random( int g ){
    int Z, k, s1, s2 ;
    s1 = Cg1[g];  s2 = Cg2[g];
    k  = s1 / 53668;
    s1 = 40014 * (s1 - k * 53668) - k * 12211;
    if ( s1 < 0 )  s1 = s1 + 2147483563 ;
    k  = s2 / 52774;
    s2 = 40692 * (s2 - k * 52774) - k * 3791;
    if ( s2 < 0 )  s2 = s2 + 2147483399 ;
    Cg1[g] = s1;  Cg2[g] = s2;
    Z = s1 - s2;
    if ( Z < 1 )  Z = Z + 2147483562 ;
    if ( Antithetic[g] )  Z = 2147483561 - Z;
    return Z ;
};

/*=================================================================*/
double Uniform( int g ) {
    double ReturnValue;
    ReturnValue = float (Random(g)) * UnSurM1 ;
    return ReturnValue ;

};

/*=================================================================*/
void CreateGenerators( int gnum){ 
    int v, i; 
    double longW; 
    float realV;
    delete(Ig1) ; delete(Ig2) ;
    delete(Lg1) ; delete(Lg2) ;
    delete(Cg1) ; delete(Cg2) ;
    delete(Antithetic); 
    /*  Memory Allocation   */ 
    NumberOfGenerators = gnum ; v = NumberOfGenerators * sizeof(int); 
    Ig1        = new int[gnum+1] ;
    Ig2        = new int[gnum+1] ; 
    Lg1        = new int[gnum+1] ; 
    Lg2        = new int[gnum+1] ;
    Cg1        = new int[gnum+1] ; 
    Cg2        = new int[gnum+1] ;
    Antithetic = new int[gnum+1] ;
    /*  v : Number of sub-sequences in each generator  */
    longW = w;
    realV = (longW -log10(NumberOfGenerators) ) / log10( 2.0 ) ;
    v = int (realV);
    /*  Calculate A1VW, A2VW  */
    A1VW = A1W ; A2VW = A2W ;
    for ( i = 1 ; i <=  v ; i++ ) {
         A1VW = MultMod( A1VW, A1VW, M1 ) ;
         A2VW = MultMod( A2VW, A2VW, M2 ) ;
        } ;
    for ( i = 1 ; i <= NumberOfGenerators ; i++) Antithetic[i] = 0;
   /*  Initialize the generators.
	   You can change the initial seeds afterwards by explicitly 
	   using the SetInitialSeed routine in your code  */
   SetInitialSeed( 1234567890, 123456789 );
};

/*=================================================================*/
int BddRandom( int g , int LB, int UB) {
int ReturnValue ;
float realUni;
   realUni = Uniform( g );
   ReturnValue = int(float( UB - LB ) * realUni )  + LB ;
   return ReturnValue;
};

/*=================================================================*/
void NewSeeds( int consume ) {
IntArrayPtr  shuffle;
int   i, temp, g, ndx ;
   shuffle = new int[NumberOfGenerators+1];
   g = consume % NumberOfGenerators ;
   AdvanceState( g, consume ) ;
    for ( i = 1 ; i <= NumberOfGenerators - 1; i++ ) {
	  shuffle[i] = BddRandom( g, i + 1, NumberOfGenerators + 1 ) ;
   } ;
    for ( i = 1 ; i <=  NumberOfGenerators - 1 ; i++ ) {
	  ndx = shuffle[i] ;
	  temp = Ig1[ndx] ;
	  Ig1[ndx] = Ig1[i] ;
	  Ig1[i] = temp ;
	  temp = Ig2[ndx] ;
	  Ig2[ndx] = Ig2[i] ;
	  Ig2[i] = temp ;
      InitGenerator( i, InitialSeed ); 
   } ;
   InitGenerator( NumberOfGenerators, InitialSeed ) ;
   delete(shuffle);
};

/*=================================================================*/
void init(){
int i; 

    ln10 = log(10.0);
/************

     By default set the number of generators to 1024. 
	 If the user needs more or less, he can use the routine 
	                      CreateGenerators( gnum ), 
	 where gnum is the number of the desired generators.

************/
   NumberOfGenerators = 1024 ;
   Ig1        = new int[NumberOfGenerators+1];
   Ig2        = new int[NumberOfGenerators+1];
   Lg1        = new int[NumberOfGenerators+1];
   Lg2        = new int[NumberOfGenerators+1];
   Cg1        = new int[NumberOfGenerators+1];
   Cg2        = new int[NumberOfGenerators+1];
   Antithetic = new int[NumberOfGenerators+1];

   A1VW = 2082007225 ;  A2VW = 784306273 ;
   /*  Initialize the Antithetic values  */
    for ( i = 1 ; i <=  NumberOfGenerators ; i++) {
      Antithetic[i] = 0;
   };

   /* Initialize the generators */
   SetInitialSeed(1234567890, 123456789);
};


// ====================================================================================

RandomStream::RandomStream(){
    count     = 0;
    statFlag  = 2;
    sum       = 0;
    sumOfSqu  = 0;
    if (start) { init(); start = 0;};
    };

IntRandomStream::IntRandomStream(){
    sampleMax = -MAX_INT;
    sampleMin = 1000000;
    };

FloatRandomStream::FloatRandomStream(){
    sampleMax = -1.0e29;
    sampleMin = 1.0e29;
    };

void IntRandomStream::Report() {
	float mean;
	float fCount = count ;
	mean = sum/count;
//	printf(" %f (avg) ", mean);
//	printf("%f (std) ", 
	    sqrt((fCount*sumOfSqu - sum*sum)/(fCount*(fCount - 1))));
//	printf("%d (count)\n ", count);
    };

void FloatRandomStream::Report() {
	float mean;
	float fCount = count ;
	mean = sum/count;
//	printf(" %f (avg) ", mean);
//	printf("%f (std) ", 
	    sqrt((fCount*sumOfSqu - sum*sum)/(fCount*(fCount - 1))));
//	printf("%d (count)\n ", count);
    };

void IntRandomStream::CollectStat(int value) {
    count++;
    sum += value;
    sumOfSqu += value*value;
    if (value > sampleMax) sampleMax = value;
    if (value < sampleMin) sampleMin = value;
    };

void FloatRandomStream::CollectStat(float value) {
    count++;
    sum += value;
    sumOfSqu += value*value;
    if (value > sampleMax) sampleMax = value;
    if (value < sampleMin) sampleMin = value;
    };

DiscreteUniform::DiscreteUniform(int n, int stream) {
    this->stream = stream;
    this->n   = n;
    statFlag  = 2; 	
    mean      = (n+1)*0.5;
    if (n == 1){
	std = 0;
    } else {
        std = sqrt((n*n-1)/(12.0));
    };
    };

int  DiscreteUniform::Draw() {
    int result;
    float sample = rint(n)*Uniform(this->stream); 
#if defined(linux ) || (defined(i386) && defined(sun))
    result = int(rint(floor(sample))) + 1;  // I hope that was the right idea
#else
    result = irint(floor(sample)) + 1;
#endif
    if (statFlag  == 2)  CollectStat(result);
    return result;
    };

void DiscreteUniform::PrintProfile() {
//	printf("\n DiscreteUniform [1-%d] generator (%f, %f)",n,mean,std);
    };

Choose::Choose(float prob, int stream) {
    this->stream = stream;
    this->prob   = prob;
    statFlag     = 2;
    mean         = prob;
    std          = sqrt(prob*(1-prob));
    };

int Choose::Draw() {
    int	sample;
    sample = (Uniform(this->stream) <= prob) ? 1 : 0;
    if (statFlag  & 2) CollectStat(sample);
    return sample;
    };

void Choose::PrintProfile() {
    //printf("\n Choose[%f] generator (%f, %f)",prob,mean,std);
    }

Uniform::Uniform(float low, float high, int stream) {
    this->stream = stream;
    this->low  = low;
    this->high = high;
    statFlag   = 2; 	
    mean       = (low+high)*0.5;
    std        = (high-low)/sqrt(12.0);
    };

float	Uniform::Draw() {
    double sample = low+(high-low)*Uniform(this->stream); 
    if (statFlag  == 2) CollectStat(sample);
    return sample;
    };

void Uniform::PrintProfile() {
	//printf("\n Uniform [%f,%f] generator (%f, %f)",low,high,mean,std);
    };

Uniform01::Uniform01(int stream) {
    this->stream = stream;
    statFlag     = 2; 	
    mean         = 0.5;
    std          = sqrt(1.0/12.0);
    };

float	Uniform01::Draw() {
    double sample = Uniform(this->stream); 
    if (statFlag  == 2) CollectStat(sample);
    return sample;
    };

void Uniform01::PrintProfile() {
//	printf("\n Uniform [0,1] generator (%f, %f)",mean,std);
    };

Exponential::Exponential(float mean, int stream) {
	this->stream = stream;
	this->mean = mean;
	this->std = mean;
	statFlag  = 2;
}

float	Exponential::Draw()
{
    float sample = -mean * ( log ( Uniform(this->stream)));
    if (statFlag  == 2) CollectStat(sample);
    return sample;
    };

void Exponential::PrintProfile() {
//	printf("\n Exponential(%f) generator (%f, %f)",mean,mean,std);
    };

HyperExp::HyperExp(float mean, float variance, int stream) 
{
	float	cx2;

	this->stream = stream;
	this->mean = mean;
	this->std = sqrt(variance);
	cx2  = variance/ (mean * mean);
	alpha = 1/(1 + cx2);
	mu1  =  mean * (1/( 1 + sqrt( 1/(alpha - 1.0)*(cx2 -1)/2)));
	mu2  =  mean * (1/( 1 - sqrt((1/(1 - alpha) - 1) * (cx2 -1)/2)));
}

HyperExp::HyperExp(float alpha, float mu1, float mu2, int stream) 
{
	this->stream = stream;
	this->alpha = alpha;
	this->mu1 = mu1;
	this->mu2 = mu2;

	mean = alpha*mu1 + (1 - alpha)*mu2;
	std = sqrt(alpha*mu1*mu1 + (1 - alpha)*mu2*mu2);
	std = sqrt(2*(alpha*mu1*mu1 + (1 - alpha)*mu2*mu2)
	        - (alpha*mu1+(1-alpha)*mu2)*(alpha*mu1+(1-alpha)*mu2));
}

float	HyperExp::Draw()
{
	float	sample;

	if (Uniform(stream) <= alpha)
	    sample = -log(Uniform(stream))*mu1;
	else	
	    sample = -log(Uniform(stream))*mu2;
	CollectStat(sample);
	return sample;
}

void	HyperExp::PrintProfile() {
//	printf("\nHyperexponential[%f, %f,%f] generator (%f, %f)",
//	    alpha,mu1,mu2,mean,std);
}

Normal::Normal(float mean, float sigma, int stream) {
	this->stream = stream;
	this->mean = mean;
	this->std = sigma;
	statFlag  = 2;
    };

float  Normal::Draw() {
    float	v1, v2;
    float	sample;

    do {
        v1 = - log(Uniform(this->stream));
        v2 = - log(Uniform(this->stream));
        } while( (2.0* v1) < ((v2 - 1.0)*(v2 - 1.0)));

    if (Uniform(this->stream) < 0.5)
        v2 *= -1;
    sample = std * v2 + mean;
	
    if (statFlag  == 2) CollectStat(sample);
    return sample;
    };
void  Normal::PrintProfile() {
//	printf("\n Normal[%f,%f] generator (%f, %f)",mean,std,mean,std);
    };

LogNormal::LogNormal(float mean, float sigma, int stream) {
	this->stream = stream;
	this->mean = mean;
	this->std = sigma;
	statFlag  = 2;
    };

float  LogNormal::Draw() {
    float	v1, v2;
    float	sample;

    do {
        v1 = - log(Uniform(this->stream));
        v2 = - log(Uniform(this->stream));
        } while( (2.0* v1) < ((v2 - 1.0)*(v2 - 1.0)));

    if (Uniform(this->stream) < 0.5)
        v2 *= -1;
    sample = exp(std * v2 + mean);
	
    if (statFlag  == 2) CollectStat(sample);
    return sample;
    };

void  LogNormal::PrintProfile() {
//	printf("\n LogNormal[%f,%f] generator (%f, %f)",mean,std,mean,std);
    };

Gamma::Gamma(float alpha, float beta, int stream) {
	this->alpha = alpha;
	this->beta  = beta;
	mean        = alpha/beta;
	std         = sqrt(alpha)/beta;
	statFlag  = 2;
}

float	GammaDist(int stream, float alpha, float beta) {
	float a, v1, v2;

	a = alpha - 1.0;
	do {
	    v1 = -log( Uniform(stream));
	    v2 = -log( Uniform(stream));
	} while ( v2 < a* (v1 - log(v1) - 1.0));

	return alpha/beta*v1;
    };

float	Gamma::Draw() {
	float sample = GammaDist(this->stream,alpha,beta);
	if (statFlag  == 2) CollectStat(sample);
	return sample;
    };

void Gamma::PrintProfile()
{
//	printf("\n Gamma [%f, %f]  generator (%f, %f)",alpha,beta,mean,std);
}

Beta::Beta(float alpha, float beta, int stream) 
{
	this->alpha  = alpha;
	this->beta  = beta;
	statFlag     = 2;
	mean = alpha/(alpha + beta);
	std = sqrt((alpha*beta / (alpha + beta + 1))/((alpha + beta)*(alpha + beta)));
}

float Beta::Draw()
{
	float	y1, y2, sample;

	y1 = GammaDist(stream,alpha,beta);
	y2 = GammaDist(stream,alpha,beta);
	sample  = y1/(y1+y2);
	if (statFlag  == 2) CollectStat(sample);
	return sample;
}

void Beta::PrintProfile() {
//    printf("\n Beta[%f, %f] generator (%f, %f)",alpha,beta,mean,std);
    };

GeneralCDF::GeneralCDF(int n, float* v, float* p , int stream) {
    int	i;

    this->n = n;
    this->v = new float[n];
    this->p = new float[n];
    q       = new float[n];

    this->v[0] = v[0];
    this->p[0] = p[0];
    this->q[0] = p[0];
    for(i=1; i < n; i++){ 
	this->v[i] = v[i];
	this->p[i] = p[i];
        this->q[i] = p[i] + p[i-1];
        }
    }

float	GeneralCDF::Draw() {
	float	sample, result;

	sample = Uniform(stream);
	result = v[binary_search(0, n-1, q, sample)];
	CollectStat(result);
	return result;
}

GeneralCDF::~GeneralCDF()
{
	delete this->v;
	delete this->p;
	delete this->q;
}

void	GeneralCDF::PrintProfile()
{
//	printf("\nGeneral CDF generator (%f, %f)",mean,std);
}

int	binary_search(int low, int high, float* q, float val)
{
	int	mid;

	while (low <= high) {
		mid = (low + high)/2;
		if (q[mid] == val) return mid;
		else if (q[mid] < val) {
			low = mid + 1;
			return binary_search(low, high, q, val);
		}
		else {
			high = mid - 1;
			return binary_search(low, high, q, val);
		}
	}
	return low;
}



