/*
*    opengl.c
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define GL_SILENCE_DEPRECATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "plane.h"

struct Pend {
	int idx; int cnt; int cpu; int gpu; int bas;
	int *siz; void **buf; GLuint hdl; GLuint tgt;
};
struct Circular {
	int head;
	int tail;
};
struct Linked {
	int list;
	int link[NUMTEXT];
	int size;
};

GLuint programId[Shaders] = {0};
GLuint blockId[Shaders] = {0};
GLuint arrayId[NUMCNTX] = {0};
GLuint vertexId[NUMCNTX] = {0};
GLuint elementId[NUMCNTX] = {0};
GLuint uniformId[NUMCNTX] = {0};
GLuint imageId[NUMTEXT] = {0};
GLsync fence[NUMCNTX] = {0}; // when to free context
int size[NUMCNTX][Memorys] = {0}; // current size of dynamic
int base[Memorys] = {0}; // offset of field in buffer
int unit[Memorys] = {0}; // padded size of field
struct Circular cntx = {0}; // arc of used contexts
struct Pend pend[NUMCNTX][NUMPEND] = {0}; // updates to use
struct Circular circ[NUMCNTX] = {0}; // arcs of used updates
struct Linked pool = {0}; // image buffers to use
struct Linked used[NUMCNTX] = {0}; // arcs of used images

void *openglBufferJ(int idx, int len, void *buf)
{
	char *ptr = buf;
	ptr += idx*len;
	return ptr;
}

void openglBufferI(int idx, int cnt, int cpu, int gpu, int bas, int *siz, void **buf, GLuint hdl, GLuint tgt)
{
	glBindBuffer(tgt, hdl);
	if (siz && *siz == 0) glBufferData(tgt, (*siz=idx+cnt)*gpu, 0, GL_STATIC_DRAW);
	if (siz && idx == 0 && cnt > *siz) glBufferData(tgt, (*siz=cnt)*gpu, 0, GL_STATIC_DRAW);
	if (siz && idx+cnt > *siz) {
	char buffer[*siz]; glGetBufferSubData(tgt, 0, (*siz)*gpu, buffer);
	glBufferData(tgt, (idx+cnt)*gpu, 0, GL_STATIC_DRAW);
	glBufferSubData(tgt, 0, (*siz)*gpu, buffer);
	*siz = idx+cnt;}
	if (siz && cpu == gpu) glBufferSubData(tgt, idx*gpu, cnt*gpu, *buf);
	if (siz && cpu != gpu) for (int i = 0; i < cnt; i++)
	glBufferSubData(tgt, (idx+i)*gpu, cpu, openglBufferJ(i,cpu,*buf));
	if (siz == 0 && cpu == gpu) glBufferSubData(tgt, bas+idx*gpu, cnt*gpu, *buf);
	if (siz == 0 && cpu != gpu) for (int i = 0; i < cnt; i++)
	glBufferSubData(tgt, bas+(idx+i)*gpu, cpu, openglBufferJ(i,cpu,*buf));
	glBindBuffer(tgt, 0);
}

void openglBufferH(struct Pend *ptr)
{
	openglBufferI(ptr->idx,ptr->cnt,ptr->cpu,ptr->gpu,ptr->bas,ptr->siz,ptr->buf,ptr->hdl,ptr->tgt);
}

void openglBufferG(struct Pend *ptr, int idx, int cnt, int cpu, int gpu, int bas, int *siz, void **buf, GLuint hdl, GLuint tgt)
{
	ptr->idx=idx;ptr->cnt=cnt;ptr->cpu=cpu;ptr->gpu=gpu;ptr->bas=bas;ptr->siz=siz;ptr->buf=buf;ptr->hdl=hdl;ptr->tgt=tgt;
}

int openglBufferF(struct Pend *ptr, int idx, int cnt, int cpu, int gpu, int bas, int *siz, void **buf, GLuint hdl, GLuint tgt)
{
	return (ptr->bas == bas && ptr->siz == siz && ptr->idx == idx && ptr->cnt == cnt);
}

void openglBuffer(int idx, int cnt, int cpu, int gpu, int bas, int *siz, void **buf, GLuint *hdl, GLuint tgt)
{
	for (int ctx = 0; ctx < NUMCNTX; ctx++) if (ctx != cntx.head) {
	int found = 0; for (int pnd = circ[ctx].head; pnd != circ[ctx].tail && !found; pnd = (pnd+1)%NUMPEND)
	if (openglBufferF(&pend[ctx][pnd],idx,cnt,cpu,gpu,bas,siz,buf,hdl[ctx],tgt)) found = 1;
	if (!found) {if ((circ[ctx].tail+1)%NUMPEND == circ[ctx].head) {
	openglBufferH(&pend[ctx][circ[ctx].head]);
	circ[ctx].head = (circ[ctx].head+1)%NUMPEND;}
	openglBufferG(&pend[ctx][circ[ctx].tail],idx,cnt,cpu,gpu,bas,siz,buf,hdl[ctx],tgt);
	circ[ctx].tail = (circ[ctx].tail+1)%NUMPEND;}}
	openglBufferI(idx,cnt,cpu,gpu,bas,siz,buf,hdl[cntx.head],tgt);
}

void openglDma()
{
	switch (cb.client->mem) {
	case (Corner): openglBuffer(cb.client->idx,cb.client->siz,sizeof(struct Vertex),sizeof(struct Vertex),0,&size[cntx.head][Corner],&cb.refer[Corner],vertexId,GL_ARRAY_BUFFER); break;
	case (Triangle): openglBuffer(cb.client->idx,cb.client->siz,sizeof(struct Facet),sizeof(struct Facet),0,&size[cntx.head][Triangle],&cb.refer[Triangle],elementId,GL_ELEMENT_ARRAY_BUFFER); break;
	case (Range): ERROR(cb.err,-1);
	case (Basis): openglBuffer(cb.client->idx,cb.client->siz,sizeof(struct Linear),unit[Basis],base[Basis],0,&cb.refer[Basis],uniformId,GL_UNIFORM_BUFFER); break;
	case (Subject): openglBuffer(0,1,sizeof(struct Affine),unit[Subject],base[Subject],0,&cb.refer[Subject],uniformId,GL_UNIFORM_BUFFER); break;
	case (Object): openglBuffer(cb.client->idx,cb.client->siz,sizeof(struct Affine),unit[Object],base[Object],0,&cb.refer[Object],uniformId,GL_UNIFORM_BUFFER); break;
	case (Feature): openglBuffer(0,1,sizeof(struct Affine),unit[Feature],base[Feature],0,&cb.refer[Feature],uniformId,GL_UNIFORM_BUFFER); break;
	case (Feather): openglBuffer(0,1,sizeof(struct Vector),unit[Feather],base[Feather],0,&cb.refer[Feather],uniformId,GL_UNIFORM_BUFFER); break;
	case (Arrow): openglBuffer(0,1,sizeof(struct Vector),unit[Arrow],base[Arrow],0,&cb.refer[Arrow],uniformId,GL_UNIFORM_BUFFER); break;
	case (Cloud): openglBuffer(cb.client->idx,cb.client->siz,sizeof(struct Vector),unit[Cloud],base[Cloud],0,&cb.refer[Cloud],uniformId,GL_UNIFORM_BUFFER); break;
	case (Hand): openglBuffer(0,1,sizeof(int),unit[Hand],base[Hand],0,&cb.refer[Hand],uniformId,GL_UNIFORM_BUFFER); break;
	case (Tag): openglBuffer(0,1,sizeof(int),unit[Tag],base[Tag],0,&cb.refer[Tag],uniformId,GL_UNIFORM_BUFFER); break;
	case (Face): ERROR(cb.err,-1);
	case (User): ERROR(cb.err,-1);
	case (Image): /*TODO*/ break;
	default: ERROR(exiterr,-1);}
}

void openglGet()
{
	float color = 0.0;
	glReadPixels(0,0,1,1,GL_RED,GL_FLOAT,&color);
	cb.state[Face]->face = color;
}

void openglFunc()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programId[cb.state[User]->user->shader]);
	glBindVertexArray(arrayId[cntx.head]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elementId[cntx.head]);
	glBindBufferBase(GL_UNIFORM_BUFFER,0,uniformId[cntx.head]);
	// if (cb.state[User]->user->shader == Display) {
	// glActiveTexture(GL_TEXTURE0+i);
	// glBindTexture(GL_TEXTURE_2D,imageId[i]);}
	for (int i = 0; i < cb.state[Range]->siz; i++) {
	void *buf = openglBufferJ(cb.state[Range]->range[i].idx,sizeof(struct Facet),0);
	cb.state[Tag]->tag = cb.state[Range]->range[i].tag;
	openglBufferI(0,1,sizeof(int),unit[Tag],base[Tag],0,&cb.refer[Tag],uniformId[cntx.head],GL_UNIFORM_BUFFER);
	glDrawElements(GL_TRIANGLES,cb.state[Range]->range[i].siz*3,GL_UNSIGNED_INT,buf);}
	fence[cntx.head] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
	if (cb.state[User]->user->shader == Display) cb.swap();
	cntx.head = (cntx.head + 1) % NUMCNTX;
	while (used[cntx.head].size > 0) {
	openglBufferH(&pend[cntx.head][used[cntx.head].list]);
	used[cntx.head].list = used[cntx.head].link[used[cntx.head].list];
	used[cntx.head].size--;}
}

int openglFull()
{
	while (cntx.tail != cntx.head) {
	GLint val;
	GLsizei len;
	glGetSynciv(fence[cntx.tail],GL_SYNC_STATUS,1,&len,&val);
	if (val == GL_UNSIGNALED) break;
	cntx.tail = (cntx.tail+1)%NUMCNTX;}
	if (cb.client)
	for (int i = 0; i < cb.client->len; i++)
	switch (cb.client->fnc[i]) {
	case (Rmw0): break;
	case (Rmw1): break;
	case (Rmw2): break;
	case (Copy): break;
	case (Save): break;
	case (Dma0):
	if (cb.client->mem == Image && cb.client->siz > pool.size) return 1; else break;
	case (Dma1): break;
	case (Draw): if (((cntx.head + 1) % NUMCNTX) == cntx.tail) return 1; else break;
	case (Port): break;
	default: ERROR(exiterr,-1);}
	return 0;
}

void openglDraw()
{
	for (int i = 0; i < cb.client->len; i++)
	switch (cb.client->fnc[i]) {
	case (Rmw0): break;
	case (Rmw1): break;
	case (Rmw2): break;
	case (Copy): break;
	case (Save): break;
	case (Dma0): openglDma(); break;
	case (Dma1): openglGet(); break;
	case (Draw): openglFunc(); break;
	case (Port): break;
	default: ERROR(exiterr,-1);}
}

void openglDone()
{
	for (int context = 0; context < NUMCNTX; context++) {
	glDeleteBuffers(1, &elementId[context]);
	glDeleteBuffers(1, &vertexId[context]);
	glDeleteVertexArrays(1, &arrayId[context]);}
	for (int shader = 0; shader < Shaders; shader++)
	glDeleteProgram(programId[shader]);
}

void openglShader(GLuint i, GLenum j, const char *file, const char *def)
{
	int stream[2] = {0};
	pipe(stream);
	if (fork() == 0) {
	const char *args[] = {"lua","opengl.lua",file,def,0};
	close(stream[0]);
	dup2(stream[1], STDOUT_FILENO);
	execvp(args[0], (char * const*)args);
	exit(-1);}
	close(stream[1]);
	char *buf[1] = {0};
	int len[1] = {0};
	int siz[1] = {0};
	int nread = 0;
	buf[0] = malloc(siz[0]=BUFSIZE*CMDSIZE);
	while ((nread = read(stream[0],buf[0]+len[0],BUFSIZE)) != 0)
	if ((len[0]+=nread)>siz[0]-BUFSIZE)
	buf[0] = realloc(buf[0],siz[0]+=BUFSIZE*CMDSIZE);
	GLuint k = glCreateShader(j);
	glShaderSource(k,1,(const char *const *)buf,len);
	glCompileShader(k);
	free(buf[0]);
	GLint stat = 0;
	glGetShaderiv(k,GL_COMPILE_STATUS,&stat);
	if (stat == GL_FALSE) {
	GLint max = 0;
	glGetShaderiv(k,GL_INFO_LOG_LENGTH,&max);
	char log[max];
	glGetShaderInfoLog(k,max,&max,log);
	printf("file(%s) log(%s)\n",file,log);}
	glAttachShader(i,k);
	glDeleteShader(k);
}

GLuint openglLoad(const char *vs, const char *gs, const char *fs)
{
	GLuint retval = glCreateProgram();
	const char *def = (gs ? "TRACK" : "DISPLAY");
	openglShader(retval,GL_VERTEX_SHADER,vs,def);
	if (gs) openglShader(retval,GL_GEOMETRY_SHADER,gs,def);
	openglShader(retval,GL_FRAGMENT_SHADER,fs,def);
	glLinkProgram(retval);
	GLint stat = 0;
	glGetProgramiv(retval,GL_LINK_STATUS,(int*)&stat);
	if (stat == GL_FALSE) {
	GLint max = 0;
	glGetProgramiv(retval,GL_INFO_LOG_LENGTH,&max);
	char log[max];
	glGetProgramInfoLog(retval,max,&max,log);
	if (gs) printf("vs(%s) gs(%s) fs(%s) log(%s)\n",vs,gs,fs,log);
	else printf("vs(%s) fs(%s) log(%s)\n",vs,fs,log);}
	return retval;
}

void openglAlign(int *total, int *base, int *unit, int elem, int count, int size)
{
	int rem = *total%elem;
	if (rem) *total += (elem-rem);
	*unit = elem*count;
	*base = *total;
	*total += *unit*size;
}

#define VERTEX(FIELD) ((void*)&(((struct Vertex *)0)->FIELD))
#define INTEGER(SIZE,FIELD) glVertexAttribIPointer(index++,SIZE,GL_INT,sizeof(struct Vertex),VERTEX(FIELD))
#define FLOATER(SIZE,FIELD) glVertexAttribPointer(index++,SIZE,GL_FLOAT,GL_FALSE,sizeof(struct Vertex),VERTEX(FIELD))
int openglInit()
{
	printf("GL_INT(%d) GL_FLOAT(%d)\n",(int)sizeof(GL_INT),(int)sizeof(GL_FLOAT));
	cb.full = openglFull;
	cb.draw = openglDraw;
	cb.done = openglDone;
	if (glewInit() != GLEW_OK) ERROR(exiterr,-1);
	glClearColor(1.00f,1.00f,1.00f,1.00f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	for (int shader = 0; shader < Shaders; shader++) {
	const char *geom = (shader == Track ? "openglg.sl" : 0);
	programId[shader] = openglLoad("openglv.sl",geom,"openglf.sl");
	blockId[shader] = glGetUniformBlockIndex(programId[shader],"Uniform");
	glUniformBlockBinding(programId[shader],blockId[shader],0);}
	int total = 0;
	openglAlign(&total,&base[Basis],&unit[Basis],4*4,3,3);
	openglAlign(&total,&base[Subject],&unit[Subject],4*4,4,1);
	openglAlign(&total,&base[Object],&unit[Object],4*4,4,NUMFILE);
	openglAlign(&total,&base[Feature],&unit[Feature],4*4,4,1);
	openglAlign(&total,&base[Feather],&unit[Feather],4*4,1,1);
	openglAlign(&total,&base[Arrow],&unit[Arrow],4*4,1,1);
	openglAlign(&total,&base[Cloud],&unit[Cloud],4*4,1,NUMFEND);
	openglAlign(&total,&base[Hand],&unit[Hand],4,1,1);
	openglAlign(&total,&base[Tag],&unit[Tag],4,1,1);
	for (int context = 0; context < NUMCNTX; context++) {
	glGenVertexArrays(1, &arrayId[context]);
	glBindVertexArray(arrayId[context]);
	glGenBuffers(1, &vertexId[context]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexId[context]);
	GLuint index = 0;
	INTEGER(3,tag[0]); // location=0 ivec3
	for (int i = 0; i < 3; i++) FLOATER(3,plane[i][0]); // location=1 vec3[3]
	INTEGER(3,versor[0]); // location=4 ivec3
	for (int i = 0; i < 3; i++) FLOATER(2,coord[i][0]); // location=5 vec2[3]
	for (int i = 0; i < 3; i++) FLOATER(4,color[i][0]); // location=8 vec4[3]
	INTEGER(3,texid[0]); // location=11 ivec3
	INTEGER(3,facid[0]); // location=12 ivec3
	INTEGER(1,matid); // location=13 int
	for (int i = 0; i < index; i++)
	glEnableVertexAttribArray(i);
	glBindVertexArray(0);
	for (int i = 0; i < index; i++)
	glDisableVertexAttribArray(i);
	glGenBuffers(1, &elementId[context]);
	glGenBuffers(1, &uniformId[context]);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformId[context]);
	glBufferData(GL_UNIFORM_BUFFER, total, 0, GL_STATIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniformId[context], 0, total);}
	glClear(GL_COLOR_BUFFER_BIT);
	cb.swap();
	return 1;
}
