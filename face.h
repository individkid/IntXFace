#include "proto.h"
struct Text {
	char **str;
	int trm;
};
struct timespec;
void debugStr(const char *str);
void readNote(eftype exc, int idx);
void readJump(eftype err, int idx);
void writeJump(eftype err, int idx);
void closeIdent(int idx);
void moveIdent(int idx0, int idx1);
int findIdent(const char *str);
int inetIdent(const char *adr, const char *num);
int openPipe();
int openFifo(const char *str);
int openFile(const char *str);
int openInet(const char *adr, const char *num);
int forkExec(const char *exe);
int pipeInit(const char *av1, const char *av2);
int puntInit(int rdx, int wdx, pftype rpf, qftype wpf);
int pselectAny(struct timespec *dly, int msk);
int waitAny();
int waitMsk(int msk);
int pauseMsk(double dly, int msk);
int pauseAny(double dly);
void waitAll();
void callInit(cftype fnc, int idx);
int pollPipe(int idx);
int pollFile(int idx);
void seekFile(long long arg, int idx);
void truncFile(int idx);
long long checkFile(int idx);
int rdlkFile(long long loc, long long siz, int idx);
int wrlkFile(long long loc, long long siz, int idx);
void unlkFile(long long loc, long long siz, int idx);
void rdlkwFile(long long loc, long long siz, int idx);
void wrlkwFile(long long loc, long long siz, int idx);
int checkRead(int idx);
int checkWrite(int idx);
void sleepSec(double sec);
void allocMark();
void allocKeep();
void allocDrop();
void allocMem(void **ptr, int siz);
void allocChr(char **ptr, int siz);
void allocInt(int **ptr, int siz);
void allocNew(long long **ptr, int siz);
void allocNum(double **ptr, int siz);
void allocOld(float **ptr, int siz);
void allocStr(char* **ptr, int siz);
void assignStr(char **ptr, const char *str);
void callStr(const char *str, int trm, int idx, void *arg);
void textStr(const char *str, int trm, int idx, void *arg);
void readStr(sftype fnc, void *arg, int idx);
void preadStr(sftype fnc, void *arg, long long loc, int idx);
void readStrHs(hftype fnc, int idx);
void readEof(int idx);
char readChr(int idx);
int readInt(int idx);
long long readNew(int idx);
double readNum(int idx);
float readOld(int idx);
int writeBuf(const void *arg, long long siz, int idx);
void flushBuf(int idx);
void writeStr(const char *arg, int trm, int idx);
void pwriteStr(const char *arg, int trm, long long loc, int idx);
void writeChr(char arg, int idx);
void writeInt(int arg, int idx);
void writeNum(double arg, int idx);
void writeNew(long long arg, int idx);
void writeOld(float arg, int idx);
void showEnum(const char *typ, const char* val, char **str, int *len);
void showStruct(const char* bef, int val, const char *aft, char **str, int *len);
void showField(const char* val, char **str, int *len);
void showOpen(const char* val, char **str, int *len);
void showClose(char **str, int *len);
void showChr(char val, char **str, int *len);
void showInt(int val, char **str, int *len);
void showNew(long long val, char **str, int *len);
void showNum(double val, char **str, int *len);
void showOld(float val, char **str, int *len);
void showStr(const char* val, char **str, int *len);
int hideIdent(const char *val, const char *str, int *len);
int hideEnum(const char* typ, const char *val, const char *str, int *len);
int hideStruct(const char* bef, int val, const char *aft, const char *str, int *len);
int hideField(const char *val, const char *str, int *len);
int hideOpen(const char *val, const char *str, int *len);
int hideClose(const char *str, int *len);
int hideChr(char *val, const char *str, int *len);
int hideInt(int *val, const char *str, int *len);
int hideNew(long long *val, const char *str, int *len);
int hideNum(double *val, const char *str, int *len);
int hideOld(float *val, const char *str, int *len);
int hideStr(char* *val, const char *str, int *len);

