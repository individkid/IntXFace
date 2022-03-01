/*
This is glob(als) and a spreadsheet with
an array of bind(ings) per row (aka l(i)st),
an array of over(loads) per collumn (aka arg(ument)),
a val(ue) (aka hash) per cell (aka self),
an id(en)t(ifier) (aka opt(ion)) per cell,
and a c(ou)nt per cell of lesser id(en)t(ifiers) in the row.
Cells in a row are filled in by calling the given function
for each collumn before starting the next row.
Typically, command line arguments are the first overload,
and command line dash option characters are the first binding.
Accessors utilHash utilOver utilTest pass their arguments to utilFind
to search for a cell in the given row aka l(i)st,
where arg(ument) is where to search from in the list,
opt(ion) is which id(en)t(ifier) to find,
c(ou)nt is number and direction of matches to find,
c(o)mp(are) is sparse relation to id(en)t(ifier) to find.
*/

#define MIN 0 // arg
#define WRD 0 // idx for over
#define PAT 0 // idx for bind
#define ICH 0 // sub for pat
#define OCH 1 // sub for pat
#define FCH 2 // sub for pat
#define RAW 0 // glb for raw
#define ENV 1 // glb for env
#define SUB 2 // glb for pipe
#define IDX 3 // glb for file
#define NUM 0 // opt for raw
#define STR 1 // opt for raw
#define OPT 2 // opt for raw
#define THD 0 // opt for file
#define INV 1 // opt for file
#define WLD -1 // opt for any
#define INP 1 // cnt for raw
#define OUT 2 // cnt for raw
#define LST -1 // cnt or cmp
#define NXT 1 // cnt or cmp
#define EQU 0 // cnt or cmp
#define JMP 4 // lim for jmp
union UtilUnion {
	int i;
	long long l;
	float f;
	double d;
	const char *s;
};
struct UtilStruct {
	int i;
	union UtilUnion u;
};
typedef struct UtilStruct (*UtilFunc)(int lst, int arg);
typedef int (*UtilComp)(int lst, int left, int right);
void utilAlloc(int lstc, int argc, int glbc);
void utilArgc(int arg, int siz);
void utilArgv(int arg, int idx, union UtilUnion val);
void utilLstc(int lst, int siz);
void utilLstv(int lst, int idx, union UtilUnion val);
void utilFunc(int lst, UtilFunc fnc);
void utilGlbv(int glb, union UtilUnion val);
void utilMerge(int lst, int size, int *index, UtilComp func);
int utilComp(int lst, int left, int right);
union UtilUnion utilUnionI(int i);
union UtilUnion utilUnionL(long long l);
union UtilUnion utilUnionS(const char *s);
struct UtilStruct utilStructI(int i, int u);
struct UtilStruct utilStructL(int i, long long u);
struct UtilStruct utilStructS(int i, const char *u);
int utilTest(int lst, int arg, int opt, int cnt, int cmp);
union UtilUnion utilOver(int lst, int arg, int opt, int cnt, int cmp, int idx);
union UtilUnion utilHash(int lst, int arg, int opt, int cnt, int cmp);
union UtilUnion utilBind(int lst, int idx);
union UtilUnion utilSelf(int arg, int idx);
union UtilUnion utilGlob(int glb);
int utilMatch(const char *lst, const char *arg);
const char *utilGetenv(const char *wrd);
int utilAtoi(const char *str);
int utilStrstr(int hay, int ndl, int ndx);
void utilFlag(int lst, const char *str);
void utilRaw(int lst, const char *str);
void utilEnv(int lst, const char *str);
void utilPipe(int lst, const char *str);
void utilFile(int lst, const char *str);
void utilUsage(int lst, const char *str);
