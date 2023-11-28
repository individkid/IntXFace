#include <stdint.h>
typedef void (*rktype)(void *dat);
typedef int (*rltype)(void **dat, const char *str);
typedef void (*cgtype)(int idx0, int idx1);
typedef int (*tltype)(int arg);
typedef void (*sftype)(char **str, int len, int idx, int loc);
typedef void (*rptype)(const char *src, int len, int idx, int loc);
typedef void (*rqtype)(int len, int idx, int loc);
typedef void (*hftype)(const char *val);
int datxSub();
void **datxDat(int sub);
int datxReadFp(int fildes, void *buf, int nbyte);
int datxWriteFp(int fildes, const void *buf, int nbyte);
void datxSplit(void **pre, void **suf, const void *dat, int len);
void datxJoin(void **dat, const void *pre, const void *suf);
int datxFind(void **val, void *key);
int datxFinds(void **val, const char *pre, const char *str);
void datxInsert(void *key, void *val, int typ);
void datxInserts(const char *pre, const char *str, void *val, int typ);
void datxNone(void **dat);
void datxChr(void **dat, char val);
void datxStr(void **dat, const char *val);
void datxInt(void **dat, int val);
void datxInt32(void **dat, int32_t val);
void datxNum(void **dat, double val);
void datxOld(void **dat, float val);
int datxChrs(void *dat);
int datxInts(void *dat);
int datxInt32s(void *dat);
int datxNums(void *dat);
int datxOlds(void *dat);
char *datxChrz(int num, void *dat);
int *datxIntz(int num, void *dat);
int32_t *datxInt32z(int num, void *dat);
double *datxNumz(int num, void *dat);
float *datxOldz(int num, void *dat);
struct Express;
int datxEval(void **dat, struct Express *exp, int typ);
void datxPrefix(const char *str);
void datxChanged(rktype fnc);
void datxCaller(rltype fnc);
void datxSetcfg(cgtype fnc);
void datxGetcfg(tltype fnc);
void datxDupstr(sftype fnc);
void datxInsstr(rptype fnc);
void datxDelstr(rqtype fnc);
void datxOutstr(hftype fnc);
