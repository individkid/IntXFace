#include "plane.h"
#include "face.h"
#include "metic.h"
#include "type.h"
#include "share.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/errno.h>
#include <string.h>

struct Kernel {
	struct Matrix compose; // optimization
	struct Matrix maintain; // change to points
	struct Matrix written; // portion written
	struct Matrix towrite; // portion to write
};
struct Kernel subject = {0};
struct Kernel *object = 0;
struct Kernel element = {0};
struct Pierce *pierce = 0;
char **strings = 0;
struct Machine *machine = 0;
int configure[Configures] = {0};
struct Client client = {0};
char collect[BUFSIZE] = {0};
int internal = 0;
int external = 0;
char *self = 0;
char *input = 0;
char *output = 0;
char *ident = 0;
int goon = 0;
uftype callDma = 0;
vftype callWake = 0;
rftype callInfo = 0;
wftype callDraw = 0;

void planeAlize(float *dir, const float *vec) // normalize
{
}
void planeCross(float *axe, const float *fix, const float *cur)
{
}
typedef void (*planeXform)(float *mat, const float *pic, const float *cor, const float *fix, const float *cur, float ang);
void planeXtate(float *mat, const float *pic, const float *cor, const float *fix, const float *cur, float ang) // rotate
{
}
void planeXlate(float *mat, const float *pic, const float *cor, const float *fix, const float *cur, float ang) // translate
{
}
void planeScale(float *mat, const float *pic, const float *cor, const float *fix, const float *cur, float ang)
{
}
void planeFocal(float *mat, const float *pic, const float *cor, const float *fix, const float *cur, float ang)
{
}
void *planeRealloc(void *ptr, int siz, int tmp, int mod)
{
	char *result = realloc(ptr,siz*mod);
	for (int i = tmp*mod; i < siz*mod; i++) result[i] = 0;
	return result;
}
int planeIndex()
{
	switch (configure[StateSelect]) {
		case (0): return 0;
		case (1): return configure[StateIndex];
		case (2): return 0;
		default: break;}
	return 0;
}
enum Memory planeMemory()
{
	switch (configure[StateSelect]) {
		case (0): return Allmatz;
		case (1): return Fewmatz;
		case (2): return Onematz;
		defalut: break;}
	return Memorys;
}
struct Kernel *planeKernel()
{
	switch (configure[StateSelect]) {
		case (0): return &subject;
		case (1): return object+planeIndex();
		case (2): return &element;
		default: break;}
	return 0;
}
planeXform planeFunc()
{
	switch (configure[StateXform]) {
		case (0): return planeXlate;
		case (1): return planeXtate;
		case (2): return planeScale;
		case (3): return planeFocal;
		default: break;}
	return 0;
}
enum Shader planeShader()
{
	switch (configure[ArgumentShader]) {
		case (0): return Dipoint;
		case (1): return Diplane;
		case (2): return Adpoint;
		case (3): return Adplane;
		case (4): return Copoint;
		case (5): return Coplane;
		default: break;}
	return Shaders;
}
struct Matrix *planeMatrix(enum Accumulate accumulate)
{
	switch (accumulate){
		case (Compose): return &planeKernel()->compose;
		case (Maintain): return &planeKernel()->maintain;
		case (Written): return &planeKernel()->written;
		case (Towrite): return &planeKernel()->towrite;
		case (OfClient): {
			struct Matrix **matrix = 0;
			enum Memory memory = planeMemory();
			switch (memory) {
				case (Allmatz): matrix = &client.all; break;
				case (Fewmatz): matrix = &client.few; break;
				case (Onematz): matrix = &client.one; break;
				default: return 0;}
			if (client.mem != memory) allocMatrix(matrix,1);
			client.cmd = configure[StateResponse];
			client.mem = memory;
			client.idx = planeIndex();
			client.siz = 1; client.slf = 0;
			return *matrix;}
		default: break;}
	return 0;
}
struct Pierce *planePierce()
{
	struct Pierce *found = 0;
	for (int i = configure[PierceIndex]; i < configure[PierceLimit]; i++) {
		struct Pierce *temp = pierce + i%configure[PierceSize];
		if (!found || (temp->vld && temp->fix[2] < found->fix[2])) found = temp;}
	return found;
}
void planeCalculate(struct Matrix *matrix)
{
	struct Vector picture = {0};
	struct Vector corner = {0};
	struct Vector fixed = {0};
	struct Vector cursor = {0};
	float angle = 0;
	picture.vec[0] = configure[WindowLeft]; picture.vec[1] = configure[WindowBase];
	corner.vec[0] = configure[WindowWide]; corner.vec[1] = configure[WindowHigh];
	fixed.vec[0] = configure[ClosestLeft]; fixed.vec[1] = configure[ClosestBase];
	cursor.vec[0] = configure[CursorLeft]; cursor.vec[1] = configure[CursorBase];
	angle = configure[CursorAngle];
	planeFunc()(matrix->mat,picture.vec,corner.vec,fixed.vec,cursor.vec,angle);
}
void planePreconfig(enum Configure cfg)
{
	switch (cfg) {
		case (RegisterDone): configure[RegisterDone] = callInfo(RegisterDone); break;
		case (RegisterOpen): configure[RegisterOpen] = callInfo(RegisterOpen); break;
		case (ClientCommand): configure[ClientCommand] = client.cmd; break;
		case (ClientMemory): configure[ClientMemory] = client.mem; break;
		case (ClientSize): configure[ClientSize] = client.siz; break;
		case (ClientIndex): configure[ClientIndex] = client.idx; break;
		case (ClientSelf): configure[ClientSelf] = client.slf; break;
		case (ClosestLeft): configure[ClosestLeft] = planePierce()->fix[0]; break;
		case (ClosestBase): configure[ClosestBase] = planePierce()->fix[1]; break;
		case (ClosestNear): configure[ClosestNear] = planePierce()->fix[2]; break;
		case (ClosestFound): configure[ClosestFound] = planePierce()->idx; break;
		case (WindowLeft): configure[WindowLeft] = callInfo(WindowLeft); break;
		case (WindowBase): configure[WindowBase] = callInfo(WindowBase); break;
		case (WindowWide): configure[WindowWide] = callInfo(WindowWide); break;
		case (WindowHigh): configure[WindowHigh] = callInfo(WindowHigh); break;
		case (CursorLeft): configure[CursorLeft] = callInfo(CursorLeft); break;
		case (CursorBase): configure[CursorBase] = callInfo(CursorBase); break;
		case (CursorAngle): configure[CursorAngle] +=/*accumulate*/ callInfo(CursorAngle); break;
		case (ButtonClick): configure[ButtonClick] = callInfo(ButtonClick); break;
		case (ButtonDrag): configure[ButtonDrag] = callInfo(ButtonDrag); break;
		case (ButtonPress): configure[ButtonPress] = callInfo(ButtonPress); break;
		case (ButtonHold): configure[ButtonHold] = callInfo(ButtonHold); break;
		default: break;}
}
void planePostconfig(enum Configure cfg, int idx)
{
	if (client.mem != Configurez) {
		freeClient(&client);
		client.cmd = configure[StateResponse];
		client.mem = Configurez; client.idx = 0; client.siz = 0;}
	if (idx >= client.siz) {
		allocConfigure(&client.cfg,idx+1);
		allocOld(&client.val,idx+1);
		client.siz = idx+1;}
	client.cfg[idx] = cfg; client.val[idx] = configure[cfg];
}
void planeReconfig(enum Configure cfg, int val)
{
	int tmp = configure[cfg];
	configure[cfg] = val;
	switch (cfg) {
		case (PierceSize): pierce = planeRealloc(pierce,val,tmp,sizeof(struct Pierce)); break;
		case (ObjectSize): object = planeRealloc(object,val,tmp,sizeof(struct Kernel)); break;
		case (StringSize): {
		for (int i = val; i < tmp; i++) free(strings[i]);
		strings = planeRealloc(strings,val,tmp,sizeof(char *));
		break;}
		case (MachineSize): machine = planeRealloc(machine,val,tmp,sizeof(struct Machine)); break;
		default: break;}
}
void planeCollect() {
	char single[2] = {0};
	int *index = 0;
	single[0] = callInfo(ButtonPress); single[1] = 0;
	index = configure + RegisterCompare;
	if (strlen(collect) < BUFSIZE-1) strcat(collect,single);
	for (*index = configure[StringIndex]; *index < configure[StringLimit]; (*index)++) {
		if (strcmp(collect,strings[(int)*index%configure[StringSize]]) == 0) break;}
}
int planeEscape(int lvl, int nxt)
{
	// TODO given old nxt and nest lines in machine return new next to get to change nest by given lvl
	return nxt;
}
int planeEval(const char *str, int arg)
{
	// TODO evaluate str to int given arg and confgure
	return 0;
}
void planeExchange(int cal, int ret)
{
	struct Machine temp = machine[cal%configure[MachineSize]];
	machine[cal%configure[MachineSize]] = machine[ret%configure[MachineSize]];
	machine[ret%configure[MachineSize]] = temp;
}
void planeBuffer() {
	switch (client.mem) {
		case (Stringz): for (int i = 0; i < client.siz; i++) assignStr(strings+(client.idx+i)%configure[StringSize],client.str[i]); break;
		case (Machinez): for (int i = 0; i < client.siz; i++) machine[(client.idx+i)%configure[MachineSize]] = client.mch[i]; break;
		case (Configurez): for (int i = 0; i < client.siz; i++) planeReconfig(client.cfg[i],client.val[i]); break;
		default: break;}
}
void *planeThread(void *arg)
{
	while (goon) {
	struct Client client = {0};
	int sub = pselectAny(0,1<<external);
	if (sub != external) break;
	if (!goon) break;
	if (!checkRead(external)) break;
	if (!checkWrite(internal)) break;
	readClient(&client,external);
	writeClient(&client,internal);
	callWake();}
	return 0;
}
void planeBoot()
{
	// TODO parse and concatenate 
	for (int i = 0; Bootstrap__Int__Str(i); i++) {
	int len = 0;
	hideClient(&client,Bootstrap__Int__Str(i),&len);
	switch (Bootstrap__Int__Process(client.mem)) {
	case (Graphics): callDma(&client); break;
	case (Central): planeBuffer(); break;
	default: break;}}
}
void planeInit(vftype init, vftype run, uftype dma, vftype wake, rftype info, wftype draw)
{
	pthread_t pthread;
	planeBoot();
	init(); // this calls planeArgument
	callDma = dma;
	callWake = wake;
	callInfo = info;
	callDraw = draw;
	internal = openPipe();
	external = pipeInit(input,output);
	if (external < 0) ERROR(exitErr,0);
	goon = 1;
	if (pthread_create(&pthread,0,planeThread,0) != 0) ERROR(exitErr,0);
	// run();
	closeIdent(internal);
	closeIdent(external);
	goon = 0;
	if (pthread_join(pthread,0) != 0) ERROR(exitErr,0);
}
void planeArgument(const char *str)
{
	if (!self) assignStr(&self,str);
	else if (!input) assignStr(&input,str);
	else if (!output) assignStr(&output,str);
	else if (!ident) assignStr(&ident,str);
	else ERROR(exitErr,0);
}
int planeConfig(enum Configure cfg)
{
	return configure[cfg];
}
void planeWake(enum Configure hint)
{
	configure[RegisterHint] = hint;
	if (configure[RegisterLine] < configure[MachineIndex] || configure[RegisterLine] >= configure[MachineLimit]) configure[RegisterLine] = configure[MachineIndex];
	while (configure[RegisterLine] >= configure[MachineIndex] && configure[RegisterLine] < configure[MachineLimit]) {
		struct Machine *mptr = machine+configure[RegisterLine]%configure[MachineSize];
		int next = configure[RegisterLine]+1;
		switch (mptr->xfr) {
			case (Read): readClient(&client,internal); break; // read internal pipe
			case (Write): writeClient(&client,external); break; // write external pipe
			case (Save): for (int i = 0; i < mptr->siz; i++) planePreconfig(mptr->cfg[i]); break; // kernel, client, pierce, or query to configure
			case (Copy): for (int i = 0; i < mptr->siz; i++) configure[mptr->cfg[i]] = configure[mptr->oth[i]]; break; // configure to configure
			case (Eval): for (int i = 0; i < mptr->siz; i++) configure[mptr->cfg[i]] = planeEval(mptr->str,mptr->val[i]); break; // script to configure
			case (Force): for (int i = 0; i < mptr->siz; i++) planeReconfig(mptr->cfg[i],mptr->val[i]); break; // machine to configure
			case (Collect): planeCollect(); break; // query to collect
			case (Setup): for (int i = 0; i < mptr->siz; i++) planePostconfig(mptr->cfg[i],mptr->val[i]); break; // configure to client
			case (Clear): identmat(planeMatrix(mptr->dst)->mat,4); break; // identity to matrix
			case (Invert): invmat(planeMatrix(mptr->dst)->mat,4); break; // invert in matrix
			case (Manip): planeCalculate(planeMatrix(mptr->dst)); break; // manip to matrix
			case (Follow): jumpmat(planeMatrix(mptr->dst)->mat,planeMatrix(mptr->src)->mat,4); break; // multiply to matrix
			case (Precede): timesmat(planeMatrix(mptr->dst)->mat,planeMatrix(mptr->src)->mat,4); break; // multiply by matrix
			case (Give): callDma(&client); break; // dma to gpu
			case (Keep): planeBuffer(); break; // dma to cpu
			case (Draw): callDraw(planeShader(),configure[ArgumentStart],configure[ArgumentStop]); break; // start shader
			case (Goto): next = planeEval(mptr->str,mptr->idx); break;
			case (Escape): next = planeEscape(planeEval(mptr->str,mptr->lvl),next); break;
			case (Nest): configure[RegisterNest] += mptr->lvl; break;
			case (Swap): planeExchange(mptr->idx,mptr->ret); break; // exchange machine lines
			default: break;}
		if (next == configure[RegisterLine]) {configure[RegisterLine] = next+1; break;}
		configure[RegisterLine] = next;
	}
}
void planeReady(struct Pierce *given, int index, int limit)
{
	for (int i = index; i < limit; i++) pierce[i%configure[PierceSize]] = given[i%configure[PierceSize]];
}
