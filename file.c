/*
*    file.c
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

#include "type.h"
#include "face.h"

#define NUMFILE 128
int face = 0;
int anonym[NUMFILE] = {0};
int number[NUMOPEN] = {0};
char *name[NUMFILE] = {0};
int named[NUMFILE] = {0};
int valid[NUMFILE] = {0};
pthread_t thread[NUMFILE] = {0};
long long identifier = 0;
jmp_buf jmpbuf[NUMFILE] = {0};
jmp_buf errbuf = {0};

#define INFINITE 1000000000ull
#define FILESIZE 1000
#define CMDSIZE 100
#define VOIDARG(x) ((void*)(((char*)(0))+(x)))
#define ARGVOID(x) (((char*)(x))-((char*)(0)))
#define NAMES 4
#define GIVEN 3
#define NAMED 2
#define HELPER 1
#define CONTROL 0

void spokerr(int arg)
{
	longjmp(jmpbuf[number[arg]],1);
}

void huberr(int arg)
{
	longjmp(errbuf,1);
}

void filerr(int arg)
{
	longjmp(jmpbuf[arg],1);
}

void exiterr(int arg)
{
	exit(-1);
}

void *file(void *arg)
{
	int idx = ARGVOID(arg);
	int given = 0;
	int control = 0;
	int helper = 0;
	int seqnum = 0;
	struct File command = {0};
	struct File response = {0};
if (setjmp(jmpbuf[idx]) == 0) {
	if ((helper = openFile(name[idx]+GIVEN)) < 0) ERROR(filerr,idx)
	number[helper] = idx; bothJump(spokerr,helper);
	writeJump(spokerr,anonym[idx]);
	readJump(spokerr,named[idx]);
	if ((given = openFile(name[idx]+GIVEN)) < 0) ERROR(filerr,idx)
	number[given] = idx; bothJump(spokerr,given);
	if ((control = openFile(name[idx]+CONTROL)) < 0) ERROR(filerr,idx)
	number[control] = idx; bothJump(spokerr,control);
	wrlkwFile(0,1,control);
	off_t config = 0;
	int stage = 0;
	while (1) {
		int handle = 0;
		if ((handle = openFile(name[idx]+GIVEN)) < 0) ERROR(filerr,idx)
		moveIdent(handle,helper);
		if (pollFile(helper)) {
			int saved = seqnum;
			seqnum = readInt(helper);
			if (seqnum != saved + 1) {config = 0; stage = 0;}
		} else {
			seqnum = seqnum + 1;
			writeInt(seqnum,helper);
		}
		unlkFile(0,1,control)
		off_t append = 0;
		int valid = 0;
		int size = 0;
		while (append < FILESIZE) {
			if (stage == 0) {
				if (!valid && pollFile(helper)) {
					readFile(&command,helper);
					valid = 1; size = 0;
					for (int i = 0; i < command.num; i++) size += command.siz[i];
				}
				if (valid && config >= command.loc + size) {
					writeFile(&command,anonym[idx]);
					valid = 0;
				} else {
					int len = readStr(given);
					if (len == 0) {
						if (valid) writeFile(&command,anonym[idx]);
						valid = 0; stage = 1; break;
					}
					config += len;
					response.idx = idx;
					response.loc = config;
					int num = 0;
					long long pos = 0;
					while (pos < len) {
						char *ptr = checkStr(pos);
						response.str[num] = ptr;
						while (pos < len && ptr[pos]) pos++;
						if (len > 0) pos++;
						response.siz[num] = pos;
						num++;
					}
					response.num = num;
					writeFile(&response,anonym[idx]);
				}
			} else {
				if (wrlkFile(append,INFINITE,helper)) {
					if (pollFile(helper)) {
						unlkFile(append,INFINITE,helper);
					} else {
						readFile(&command,named[idx]);
						for (int i = 0; i < command.num; i++) {
							writeStr(command.str[i],command.siz[i],given);
						}
						writeFile(&command,helper);
						unlckFile(append,INFINITE,helper);
						append += sizeFile(&command);
					}
				} else {
					rdlckwFile(append,1,helper);
					if (pollFile(helper)) {
						readFile(&command,helper);
						unlckFile(append,1,helper);
						append += sizeFile(&command);
					} else {
						unlckFile(append,1,helper);
					}
				}
				writeFile(&command,anonym[idx]);
				if (command.num == 0 && command.loc == identifier) {
					closeIdent(helper);
					closeIdent(given);
					closeIdent(control);
					return 0;
				}
			}
		}
		wrlkwFile(0,1,control);
		if (unlink(name[idx]+HELPER) < 0) ERROR(filerr,idx)
	}
} else {
	closeIdent(helper);
	closeIdent(given);
	closeIdent(control);
	writeJump(0,anonym[idx]);
	command.num = 0;
	writeFile(&command,anonym[idx]);
}
	return 0;
}

void cleanup(int sub)
{
	if (pthread_join(thread[sub],0) < 0) ERROR(huberr,-1)
	closeIdent(anonym[sub]);
	closeIdent(named[sub]);
	valid[sub] = 0;
}

void finish(int sub)
{
	struct File command = {0};
	command.num = 0;
	command.loc = identifier;
	writeFile(&command,named[sub]);
	cleanup(sub);
}

int main(int argc, char **argv)
{
	if (argc != 4) return -1;
	struct File command = {0};
	// TODO identifier = pid + seconds-since-publish
	if ((face = pipeInit(argv[1],argv[2])) < 0) return -1;
if (setjmp(errbuf) == 0) {
	bothJump(huberr,face);
	for (int sub = waitAny(); sub >= 0; sub = waitAny()) {
		readFile(&command,sub);
		if (sub == face && command.idx >= 0 && command.idx < NUMFILE && !valid[command.idx]) {
			valid[command.idx] = 1;
			if (command.num != 1) ERROR(huberr,-1)
			if ((anonym[command.idx] = openPipe()) < 0) ERROR(huberr,-1)
			if ((name[command.idx] = malloc(strlen(command.str[0])+NAMES)) == 0) ERROR(huberr,-1)
			for (int i = 0; i < NAMES-1; i++) name[command.idx][i] = '.';
			strcpy(name[command.idx]+GIVEN,command.str[0]);
			if ((named[command.idx] = addFifo(name[command.idx]+NAMED)) < 0) ERROR(huberr,-1)
			number[named[command.idx]] = command.idx; writeJump(huberr,named[command.idx]);
			number[anonym[command.idx]] = command.idx; readJump(huberr,anonym[command.idx]);
			if (pthread_create(&thread[command.idx],0,file,VOIDARG(command.idx)) < 0) ERROR(huberr,-1)
		} else if (sub == face && command.idx >= 0 && command.idx < NUMFILE && command.num == 0) {
			finish(command.idx);
		} else if (sub == face && command.num == 0) {
			for (int i = 0; i < NUMFILE; i++) if (valid[i]) finish(i);
			return 0;
		} else if (sub == face) {
			if (command.idx < 0) ERROR(huberr,-1)
			if (command.idx >= NUMFILE) ERROR(huberr,-1)
			writeFile(&command,named[command.idx]);
		} else {
			command.idx = number[sub];
			if (command.num == 0) cleanup(command.idx);
			writeFile(&command,face);
		}
	}
} else {
	writeJump(0,face);
	command.idx = -1;
	command.num = 0;
	writeFile(&command,face);
}
	return -1;
}
