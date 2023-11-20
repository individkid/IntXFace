#include "proto.h"
struct timespec;
typedef int (*fgtype)(const char *str, int len, int idx);
typedef void (*chtype)();
typedef void (*hgtype)(int val);
typedef void (*eftype)(const char *str, int num, int idx);
typedef int (*pftype)(int fildes, void *buf, int nbyte);
typedef int (*qftype)(int fildes, const void *buf, int nbyte);
typedef void (*chtype)();
typedef void (*hktype)(int len, const char *val);
typedef void (*hftype)(const char *val);
typedef void (*hmtype)(char val);
typedef void (*hltype)(int32_t val);
typedef void (*hitype)(long long val);
typedef void (*hjtype)(float val);
typedef void (*hhtype)(double val);
void termFunc(fgtype fnc);
void intrFunc(chtype fnc);
void noteFunc(hgtype fnc);
void errFunc(eftype fnc);
void noteFuncLua(const char *str);
void errFuncLua(const char *str);
void callIntr();
void callNote(int idx);
void callErr(int idx);
void closeIdent(int idx);
void moveIdent(int idx0, int idx1);
int findIdent(const char *str);
int inetIdent(const char *adr, const char *num);
void **userIdent(int idx);
int openPipe();
int openSide();
int openFifo(const char *str);
int openName(const char *str);
int openFile(const char *str);
int openInet(const char *adr, const char *num);
int forkExec(const char *exe);
int pipeInit(const char *av1, const char *av2);
int openFork();
int openCheck(int idx);
int openRdfd(int idx);
int openWrfd(int idx);
int openExec(const char *exe, const char *arg);
int rdwrInit(int rdfd, int wrfd);
int rdfdInit(int rdfd, int hint);
int wrfdInit(int wrfd, int hint);
int puntInit(int rdx, int wdx, pftype rpf, qftype wpf);
int buntInit(int rfd, int wfd, pftype rpf, qftype wpf);
int waitRead(double dly, int msk);
int waitExit(); // TODO add dly and msk
void callInit(hgtype fnc, int idx);
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
void allocChr(char **ptr, int siz);
void allocInt(int **ptr, int siz);
void resizeInt(int **ptr, int sav, int siz);
void appendInt(int **ptr, int val, int *siz);
void allocInt32(int32_t **ptr, int siz);
void allocNew(long long **ptr, int siz);
void allocNum(double **ptr, int siz);
void allocOld(float **ptr, int siz);
void allocStr(char* **ptr, int siz);
void assignStr(char **ptr, const char *str);
void allocDat(void* **ptr, int siz);
void assignDat(void **ptr, const void *dat);
int readBuf(void *arg, long long siz, int idx);
void readStr(char **str, int idx);
void preadStr(char **str, long long loc, int idx);
void readDat(void **dat, int idx);
void readEof(int idx);
char readChr(int idx);
int readInt(int idx);
int32_t readInt32(int idx);
long long readNew(int idx);
double readNum(int idx);
float readOld(int idx);
int writeBuf(const void *arg, long long siz, int idx);
void flushBuf(int idx);
void writeStr(const char *arg, int idx);
void pwriteStr(const char *arg, long long loc, int idx);
void writeDat(const void *dat, int idx);
void writeChr(char arg, int idx);
void writeInt(int arg, int idx);
void writeInt32(int32_t arg, int idx);
void writeNum(double arg, int idx);
void writeNew(long long arg, int idx);
void writeOld(float arg, int idx);
void showEnum(const char *typ, const char* val, char **str);
void showField(const char* val, char **str, int arg, ...);
void showOpen(const char* val, char **str);
void showClose(char **str);
void showChr(char val, char **str);
void showInt(int val, char **str);
void showInt32(int32_t val, char **str);
void showNew(long long val, char **str);
void showNum(double val, char **str);
void showOld(float val, char **str);
void showStr(const char* val, char **str);
void showDat(const void* val, char **str);
int hideIdent(const char *val, const char *str, int *len);
int hideEnum(const char* typ, const char *val, const char *str, int *len);
int hideOpen(const char *val, const char *str, int *siz);
int hideClose(const char *str, int *siz);
int hideField(const char *val, const char *str, int *siz, int arg, ...);
int hideChr(char *val, const char *str, int *len);
int hideInt(int *val, const char *str, int *len);
int hideInt32(int32_t *val, const char *str, int *len);
int hideNew(long long *val, const char *str, int *len);
int hideNum(double *val, const char *str, int *len);
int hideOld(float *val, const char *str, int *len);
int hideStr(char* *val, const char *str, int *len);
int hideDat(void **val, const char *str, int *siz);
void readStrHs(hftype fnc, int idx);
void readDatHs(hktype fnc, int idx);
void writeDatHs(int len, const char *val, int idx);
int hideFieldV(const char *val, const char *str, int *siz, int arg, int *sub);
int hideEnumHs(const char *typ, const char *val, const char *str, hftype fnc);
int hideOpenHs(const char *typ, const char *str, hftype fnc);
int hideCloseHs(const char *str, hftype fnc);
int hideFieldHs(const char *typ, int arg, int *ptr, const char *str, hftype fnc);
int hideStrHs(hftype val, const char *str, hftype fnc);
int hideChrHs(hmtype val, const char *str, hftype fnc);
int hideIntHs(hgtype val, const char *str, hftype fnc);
int hideInt32Hs(hltype val, const char *str, hftype fnc);
int hideNumHs(hhtype val, const char *str, hftype fnc);
int hideNewHs(hitype val, const char *str, hftype fnc);
int hideOldHs(hjtype val, const char *str, hftype fnc);
void showFieldV(const char* val, char **str, int arg, int *sub);
void showEnumHs(const char *typ, const char *val, const char *str, hftype fnc);
void showOpenHs(const char *typ, const char *str, hftype fnc);
void showCloseHs(const char *str, hftype fnc);
void showFieldHs(const char *typ, int arg, int *ptr, const char *str, hftype fnc);
void showStrHs(const char *val, const char *str, hftype fnc);
void showChrHs(char val, const char *str, hftype fnc);
void showIntHs(int val, const char *str, hftype fnc);
void showInt32Hs(int32_t val, const char *str, hftype fnc);
void showNumHs(double val, const char *str, hftype fnc);
void showNewHs(long long val, const char *str, hftype fnc);
void showOldHs(float val, const char *str, hftype fnc);

