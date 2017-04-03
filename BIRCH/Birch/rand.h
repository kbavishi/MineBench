/****************************************************************************
     Copyright (c) 1993 1994 1995
     By Miron Livny, Madison, Wisconsin
     All Rights Reserved.

     UNDER NO CIRCUMSTANCES IS THIS PROGRAM TO BE COPIED OR DISTRIBUTED
     WITHOUT PERMISSION OF MIRON LIVNY

     obtained and modified 3/15/96 by Tian Zhang 
****************************************************************************/

#ifndef RAND_H
#define RAND_H 

/******************************************************************************

	                    rand.h

******************************************************************************/
/* header file containing class description of the various 
 * random number generators.
 */

#define	MAX_OBJECTS	30

/* class RandomGenerator
 * generates an integer in [0,max_int]
 */

class RandomStream {
    public:
        int     stream;	        // stream
        int     count;	        // numbers generated
        short	statFlag;
        float	mean, std;	// of the distribution being generated
	float   sum;
	float   sumOfSqu;

	RandomStream();
        virtual  void	Report() = 0;	     //prints max, min, count etc.
        virtual  void	PrintProfile() = 0 ; //prints the  type of generator.

        int 	SampleSize()    { return count;} ;
        float	Mean()	        { return mean;}
        float	STD()	        { return std;}
			    
        float	SampleMean();	
        float	SampleVar();
    };

class IntRandomStream : public RandomStream
    {
    public:
	int sampleMax;
	int sampleMin;
	
	IntRandomStream();
	virtual int Draw() = 0 ;
        int	SampleMax()	{ return sampleMax;}
        int	SampleMin()	{ return sampleMin;}
        void	CollectStat(int value);	 //prints max, min, count etc.
        virtual void	Report();	 //prints max, min, count etc.
    };

class  DiscreteUniform  : public IntRandomStream{
public:
	int n;
	DiscreteUniform(int n, int stream);
	int	Draw();
        virtual  void	PrintProfile(); //prints the  type of generator.
};

/* class Choose
 * returns true(1) with probability p.
 */
class Choose : public IntRandomStream {
public:
	float prob;
	Choose(float prob,int stream);
	int   Draw();	
	void  PrintProfile();
};

class FloatRandomStream : public RandomStream
    {
    public:
	float sampleMax;
	float sampleMin;
	
	FloatRandomStream();
	virtual float Draw() = 0 ;
        float	SampleMax()	{ return sampleMax;}
        float	SampleMin()	{ return sampleMin;}
        void	CollectStat(float value);//prints max, min, count etc.
        virtual void	Report();	 //prints max, min, count etc.
    };


/* class Uniform01  returns a float in [0,1] */

class  Uniform01  : public FloatRandomStream{
public:
	Uniform01(int stream);
	float	Draw();
        virtual  void	PrintProfile(); //prints the  type of generator.
};

class  Uniform  : public FloatRandomStream{
public:
	float low ;
	float high;
	Uniform(float low, float high,int stream);
	float	Draw();
        virtual  void	PrintProfile(); //prints the  type of generator.
};

class  Exponential  : public FloatRandomStream{
    public:
	Exponential(float mean,int stream);
	float	Draw();
        virtual  void	PrintProfile(); //prints the  type of generator.
    };


class  HyperExp  : public FloatRandomStream {
private:
	float	alpha;
	float	mu1, mu2;
public:
	HyperExp(float mean, float variance,int stream);
	HyperExp(float alpha, float mu1, float mu2,int stream);

	float	Draw();
	void	PrintProfile();
    };

class  Normal  : public FloatRandomStream{
    public:
	Normal(float mean, float sigma, int stream);
	float	Draw();
        virtual  void	PrintProfile(); //prints the  type of generator.
    };

class  LogNormal  : public FloatRandomStream{
    public:
	LogNormal(float mean, float sigma, int stream);
	float	Draw();
        virtual  void	PrintProfile(); //prints the  type of generator.
    };

class Gamma : public FloatRandomStream {
    public:
	Gamma(float alpha, float beta,int stream);
	float alpha;
	float beta;
	float Draw();
	void  PrintProfile();
};

class Beta : public FloatRandomStream {
public:

	float alpha;
	float beta;

	Beta(float alpha, float beta, int stream);
	float Draw();
	void  PrintProfile();
};

class GeneralCDF : public FloatRandomStream {
private:
	float*  q;	
public:

	int   n;
	float *v;
	float *p;

	GeneralCDF(int n, float* v, float* p,int stream);
	virtual ~GeneralCDF() ;

	float	Draw();
	void	PrintProfile();
	
    };

#endif RAND_H

