#include <math.h>

float dotvec(float *u, float *v, int n);
float *plusvec(float *u, float *v, int n);
float *scalevec(float *u, float s, int n);
float *jumpvec(float *u, float *v, int n);
float *zerovec(float *u, int n);
float *normvec(float *u, int n);
float *unitvec(float *u, int n, int m);
float *timesmat(float *u, float *v, int n);
float *jumpmat(float *u, float *v, int n);
float *argmat(int n, int m, float *u, ...);
float *identmat(float *u, int n);
float *copyary(float *u, float *v, int duty, int stride, int size);
float *copyvec(float *u, float *v, int n);
float *copymat(float *u, float *v, int n);
float *compmat(float *u, float *v, int n);
float *crossmat(float *u);
float *crossvec(float *u, float *v);
float *submat(float *u, int i, int n);
float *minmat(float *u, int n);
float *cofmat(float *u, int n);
float detmat(float *u, int n);
float *xposmat(float *u, int n);
float *adjmat(float *u, int n);
float *invmat(float *u, int n);
float *tweakvec(float *u, float a, float b, int n);
float absval(float a);
