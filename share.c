#include "argx.h"
#include "memx.h"
#include "luax.h"
#include "face.h"
#include "type.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <libgen.h>

int faces = 0;
int type = 0;
int field = 0;
int iface = 0;
int oface = 0;
int misc = 0;
int zero = 0;

void shareRunA(void **run, void *use)
{
	nestScan();
	memxCopy(run,use);
}
void shareRunCF(const char *str, int trm, int idx, void *arg)
{
	void **mem = (void**)arg;
	memxInit(mem,str);
}
void shareRunCG(void **run)
{
	void *que = argxRun(misc);
	memxCopy(run,memxSkip(que,0));
	memxDel(&que,0);
}
void shareRunCH(void **run)
{
	void *que = argxRun(misc);
	memxCopy(run,memxSkip(que,memxSize(que)-1));
	memxDel(&que,memxSize(que)-1);
}
void shareRunCI(void **run)
{
	void *que = argxRun(misc);
	memxAdd(&que,*run,0);
}
void shareRunCJ(void **run)
{
	void *que = argxRun(misc);
	memxAdd(&que,*run,memxSize(que));
}
void shareRunC(void **run, void *use)
{
	int len = memxSize(use);
	int rfd = memxOpen(run);
	int ifd = memxInt(argxRun(iface));
	int ofd = memxInt(argxRun(oface));
	int typ = memxInt(memxSkip(argxRun(type),0));
	int fld = memxInt(memxSkip(argxRun(type),1));
	int idx = memxInt(memxSkip(argxRun(type),2));
	void *tmp = memxTemp(0);
	int tfd = memxOpen(tmp);
	for (int i = 0; i < memxSize(use); i++) {
	void *mem = memxSkip(use,i);
	switch ((enum Stream) memxInt(mem)) {
	// read to type from type
	case (RdTypP): loopStruct(typ,ifd,rfd); break;
	case (RdTypHd): shareRunCG(run); break;
	case (RdTypTl): shareRunCH(run); break;
	// read to string from raw
	case (RdStrP): readStruct(shareRunCF,(void*)run,typ,ifd); break;
	case (RdStrHd): shareRunCG(run); readStruct(shareRunCF,(void*)run,typ,rfd); break;
	case (RdStrTl): shareRunCH(run); readStruct(shareRunCF,(void*)run,typ,rfd); break;
	// read to raw from string
	case (RdRawP): loopStruct(typ,ifd,rfd); writeStruct(memxStr(*run),typ,rfd); break;
	case (RdRawHd): shareRunCG(run); writeStruct(memxStr(*run),typ,rfd); break;
	case (RdRawTl): shareRunCH(run); writeStruct(memxStr(*run),typ,rfd); break;
	// read to field in type
	case (RdFldP): readField(typ,fld,idx,rfd,ifd,rfd); break;
	case (RdFldHd): shareRunCG(&tmp); readField(typ,fld,idx,rfd,tfd,rfd); break;
	case (RdFldTl): shareRunCH(&tmp); readField(typ,fld,idx,rfd,tfd,rfd); break;
	// write from type to type
	case (WrTypP): loopStruct(typ,rfd,ofd); break;
	case (WrTypHd): shareRunCI(run); break;
	case (WrTypTl): shareRunCJ(run); break;
	// write from string to raw
	case (WrStrP): writeStruct(memxStr(*run),typ,ofd); break;
	case (WrStrHd): writeStruct(memxStr(*run),typ,tfd); shareRunCI(tmp); break;
	case (WrStrTl): writeStruct(memxStr(*run),typ,tfd); shareRunCJ(tmp); break;
	// write from raw to string
	case (WrRawP): readStruct(shareRunCF,&tmp,typ,rfd); writeStr(memxStr(tmp),1,ofd); break;
	case (WrRawHd): readStruct(shareRunCF,&tmp,typ,rfd); shareRunCI(tmp); break;
	case (WrRawTl): readStruct(shareRunCF,&tmp,typ,rfd); shareRunCJ(tmp); break;
	// write from field in type
	case (WrFldP): writeField(typ,fld,idx,rfd,ofd); break;
	case (WrFldHd): writeField(typ,fld,idx,rfd,tfd); shareRunCI(tmp); break;
	case (WrFldTl): writeField(typ,fld,idx,rfd,tfd); shareRunCJ(tmp); break;
	default: ERROR();}}
}
int shareRunD(void *use)
{
	int msk, dly, val;
	msk = memxMask(argxRun(faces));
	dly = memxInt(use);
	if (dly == 0) val = waitRead(0.0,msk);
	else val = waitRead(dly,msk);
	return val;
}
int shareUseLF(int idx, void *buf, int nbyte)
{
	void *mem = memxSkip(argxUse(idx),1);
	if (luaxCall(memxStr(mem),protoClosePf(idx,nbyte)) < 0) ERROR();
	return protoResultPf(buf);
}
int shareUseLG(int idx, const void *buf, int nbyte)
{
	void *mem = memxSkip(argxUse(idx),2);
	if (luaxCall(memxStr(mem),protoCloseQf(idx,buf,nbyte)) < 0) ERROR();
	return protoResultQf();
}
void shareUseL(void **use, const char *str)
{
	void *tmp0 = 0; void *tmp1 = 0;
	memxInit(use,"");
	memxForm(&tmp0,"%d",puntInit(argxHere(),argxHere(),shareUseLF,shareUseLG)); memxInit(&tmp1,str);
	memxList(use,tmp0); memxList(use,tmp1);
	memxMake(use,tmp0); memxMake(use,tmp1);}
void shareRunL(void **run, void *use)
{
	memxCopy(run,memxSkip(use,0));
}
int shareUseP(const char *str)
{
	void *mem = memxTemp(0); memxInit(&mem,str); return rdfdInit(memxInt(mem),memxInt(argxUse(oface)));
}
int shareUseQ(const char *str)
{
	void *mem = memxTemp(0); memxInit(&mem,str); return wrfdInit(memxInt(mem),memxInt(argxUse(iface)));
}
int shareLuax(const char *str)
{
	if (strcmp(str,"faces") == 0) return faces;
	else if (strcmp(str,"type") == 0) return type;
	else if (strcmp(str,"field") == 0) return field;
	else if (strcmp(str,"iface") == 0) return iface;
	else if (strcmp(str,"oface") == 0) return oface;
	else if (strcmp(str,"misc") == 0) return misc;
	else if (strcmp(str,"zero") == 0) return zero;
	ERROR();
	return 0;
}
int main(int argc, char **argv)
{
	if (luaxFile("type.lua") < 0) {protoErr("shareC: cannot load library: type.lua\n"); fprintf(stderr,"%s",protoMsg()); return -1;}
	luaxAdd("shareLuax",protoTypeFf(shareLuax));
	luaxAdd("argxUse",protoTypeTf(argxUse));
	luaxAdd("argxRun",protoTypeTf(argxRun));
	memxLuax();
	faces = getLocation();
	type = getLocation();
	iface = getLocation();
	oface = getLocation();
	misc = getLocation();
	zero = getLocation();
	addFlow("a",protoTypeNf(memxInit),protoTypeMf(shareRunA));
	addFlow("b",protoTypeNf(memxInit),protoTypeMf(memxCopy));
	addFlow("c",protoTypeNf(memxInit),protoTypeMf(shareRunC));
	addFlow("d",protoTypeNf(memxInit),protoTypeOf(shareRunD));
	addFlow("e",protoTypeFf(forkExec),protoTypeMf(memxCopy));
	addFlow("f",protoTypeFf(openFile),protoTypeMf(memxCopy));
	addFlow("g",protoTypeFf(openFifo),protoTypeMf(memxCopy));
	addFlow("h",protoTypeGf(openInet),protoTypeMf(memxCopy));
	addFlow("i",protoTypeNf(memxInit),protoTypeMf(argxCopy));
	addJump("j",protoTypeNf(memxInit),protoTypeOf(argxJump));
	addNest("k",protoTypeNf(memxInit),protoTypeMf(memxCopy));
	addFlow("l",protoTypeNf(shareUseL),protoTypeMf(shareRunL));
	addFlow("m",protoTypeNf(memxInit),protoTypeMf(argxKeep));
	addFlow("n",protoTypeNf(memxInit),protoTypeMf(argxCopy));
	addFlow("o",protoTypeNf(memxInit),protoTypeMf(argxCopy));
	addFlow("p",protoTypeFf(shareUseP),protoTypeMf(memxCopy));
	addFlow("q",protoTypeFf(shareUseQ),protoTypeMf(memxCopy));
	mapCallback("b",type,protoTypeMf(memxCopy));
	mapCallback("diefghlp",iface,protoTypeMf(memxCopy));
	mapCallback("efghloq",oface,protoTypeMf(memxCopy));
	mapCallback("efghlpq",faces,protoTypeMf(memxList));
	mapCallback("mn",faces,protoTypeMf(memxCopy));
	mapDefault("i",iface,protoTypeMf(memxCopy));
	mapDefault("o",oface,protoTypeMf(memxCopy));
	mapDefault("m",zero,protoTypeMf(memxCopy));
	mapDefault("n",faces,protoTypeMf(memxCopy));
	mapContext("p",iface,protoTypeMf(memxCopy));
	mapContext("q",oface,protoTypeMf(memxCopy));
	for (int i = 1; i < argc; i++) useArgument(argv[i]);
	runProgram();
	return 0;
}
