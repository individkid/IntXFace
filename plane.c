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
	int valid; // optimized
	struct Matrix compose; // optimization
	struct Matrix maintain; // change to points
	struct Matrix written; // portion written
	struct Matrix towrite; // portion to write
	struct Vector fixed; // fixed point
};
struct Kernel subject = {0};
struct Kernel *object = 0;
struct Kernel element = {0};
struct Pierce *pierce = 0;
char **strings = 0;
struct Machine *machine = 0;
int configure[Configures] = {0};
char collect[BUFSIZE] = {0};
int internal = 0;
int external = 0;
const char *input = 0;
const char *output = 0;
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
struct Matrix **planeMatrix(struct Client *client)
{
	switch (configure[StateSelect]) {
		case (0): return &client->all;
		case (1): return &client->few;
		case (2): return &client->one;
		defalut: break;}
	return 0;
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
		case (3): return planeFocal; // TODO add normalize step to shaders
		default: break;}
	return 0;
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
	cursor.vec[0] = configure[CursorLeft]; cursor.vec[1] = configure[CursorBase];
	angle = configure[CursorAngle];
	copymat(matrix->mat,planeKernel()->compose.mat,4);
	copyvec(fixed.vec,planeKernel()->fixed.vec,4);
	planeFunc()(matrix->mat,picture.vec,corner.vec,fixed.vec,cursor.vec,angle);
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
void *planeRealloc(void *ptr, int siz, int tmp, int mod)
{
	char *result = realloc(ptr,siz*mod);
	for (int i = tmp*mod; i < siz*mod; i++) result[i] = 0;
	return result;
}
void planeReconfig(enum Configure cfg, float val)
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
struct Pierce *planePierce()
{
	struct Pierce *found = 0;
	for (int i = configure[PierceIndex]; i < configure[PierceLimit]; i++) {
		struct Pierce *temp = pierce + i%configure[PierceSize];
		if (!found || (temp->vld && temp->fix[2] < found->fix[2])) found = temp;}
	return found;
}
void *planeThread(void *arg)
{
	while (goon) {
	struct Client client = {0};
	int sub = pselectAny(0,1<<external);
	if (sub != external) ERROR(exitErr,0);
	readClient(&client,external);
	writeClient(&client,internal);
	callWake();}
	return 0;
}
void planeInit(vftype init, vftype run, uftype dma, vftype wake, rftype info, wftype draw)
{
	pthread_t pthread;
	init(); // this calls planeArgument
	callDma = dma;
	callWake = wake;
	callInfo = info;
	callDraw = draw;
	internal = openPipe();
	external = pipeInit(input,output);
	goon = 1;
	if (pthread_create(&pthread,0,planeThread,0) != 0) ERROR(exitErr,0);
	run();
	goon = 0;
	if (pthread_join(pthread,0) != 0) ERROR(exitErr,0);
}
void planeArgument(const char *str)
{
	if (!input) input = str;
	else if (!output) output = str;
	else ERROR(exitErr,0);
}
int planeConfig(enum Configure cfg)
{
	return configure[cfg];
}
void planeWake(enum Configure hint)
{
	struct Client client = {0};
	char collect[BUFSIZE] = {0};
	configure[RegisterLine] = configure[MachineIndex];
	configure[RegisterHint] = hint;
	while (configure[RegisterLine] >= configure[MachineIndex] && configure[RegisterLine] < configure[MachineLimit]) {
		struct Machine *mptr = machine+configure[RegisterLine]%configure[MachineSize];
		int next = configure[RegisterLine]+1;
		switch (mptr->xfr) {
			case (Read): readClient(&client,internal); break; // read internal pipe
			case (Write): writeClient(&client,external); break; // write external pipe
			case (Save): { // kernel, client, pierce, or query to configure
			switch (mptr->cfg) {
				case (RegisterValid): configure[RegisterValid] = planeKernel()->valid; break;
				case (RegisterDone): configure[RegisterDone] = callInfo(RegisterDone); break;
				case (ClientCommand): configure[ClientCommand] = client.cmd; break;
				case (ClientMemory): configure[ClientMemory] = client.mem; break;
				case (ClientSize): configure[ClientSize] = client.siz; break;
				case (ClientIndex): configure[ClientIndex] = client.idx; break;
				case (ClientSelf): configure[ClientSelf] = client.slf; break;
				case (PierceLeft): configure[PierceLeft] = planePierce()->fix[0]; break;
				case (PierceBase): configure[PierceBase] = planePierce()->fix[1]; break;
				case (PierceNear): configure[PierceNear] = planePierce()->fix[2]; break;
				case (PierceFound): configure[PierceFound] = planePierce()->idx; break;
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
			break;}
			case (Copy): configure[mptr->cfg] = configure[mptr->oth]; break; // configure to configure
			case (Force): planeReconfig(mptr->cfg,mptr->val); break; // machine to configure
			case (Collect): { // query to collect
			char single[2] = {0};
			int *index = 0;
			single[0] = callInfo(ButtonPress); single[1] = 0;
			index = configure + RegisterCompare;
			if (strlen(collect) < BUFSIZE-1) strcat(collect,single);
			for (*index = configure[StringIndex]; *index < configure[StringLimit]; (*index)++) {
				if (strcmp(collect,strings[(int)*index%configure[StringSize]]) == 0) break;}
			break;}
			case (Setup): { // configure to client
			if (client.mem != Configurez) {
				freeClient(&client);
				client.cmd = configure[StateResponse];
				client.mem = Configurez; client.idx = 0; client.siz = 0;}
			if (mptr->idx >= client.siz) {
				allocConfigure(&client.cfg,mptr->idx+1);
				allocOld(&client.val,mptr->idx+1);
				client.siz = mptr->idx+1;}
			client.cfg[mptr->idx] = mptr->cfg; client.val[mptr->idx] = configure[mptr->cfg];
			break;}
			case (Manip): { // kernel to client
			client.cmd = configure[StateResponse];
			client.mem = planeMemory();
			client.idx = planeIndex();
			client.siz = 1; client.slf = 0;
			allocMatrix(planeMatrix(&client),1);
			planeCalculate(*planeMatrix(&client));
			break;}
			case (Lead): break; // configure to kernel TODO maintain followed by calculate to maintain, change fixed, clear valid
			case (Merge): break; // echo to kernel TODO written followed by towrite to written, clear towrite, maintain followed by read to maintain, clear valid
			case (Follow): break; // other to kernel TODO maintain followed by read to maintain, clear valid
			case (Compose): break; // make kernel valid TODO maintain followed by inverse written to compose, set valid
			case (Give): callDma(&client); break; // dma to gpu
			case (Keep): { // dma to cpu
			switch (client.mem) {
				case (Stringz): for (int i = 0; i < client.siz; i++) assignStr(strings+(client.idx+i)%configure[StringSize],client.str[i]); break;
				case (Machinez): for (int i = 0; i < client.siz; i++) machine[(client.idx+i)%configure[MachineSize]] = client.mch[i]; break;
				case (Configurez): for (int i = 0; i < client.siz; i++) planeReconfig(client.cfg[i],client.val[i]); break;
				default: break;}
			break;}
			case (Draw): callDraw(planeShader(),configure[ArgumentStart],configure[ArgumentStop]); break; // start shader
			case (Equal): if (configure[mptr->cfg] == mptr->val) next = mptr->idx; break; // jump if equal
			case (Noteq): if (configure[mptr->cfg] != mptr->val) next = mptr->idx; break; // jump not equal
			case (Less): if (configure[mptr->cfg] < mptr->val) next = mptr->idx; break; // jump if less
			case (More): if (configure[mptr->cfg] > mptr->val) next = mptr->idx; break; // jump if more
			case (Goto): next = mptr->idx; break; // jump regardless
			default: break;}
		configure[RegisterLine] = next;}
}
void planeReady(struct Pierce *given, int size)
{
	for (int i = 0; i < size; i++) pierce[(given->idx+i)%configure[PierceSize]] = given[i];
}
