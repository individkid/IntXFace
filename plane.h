enum Micro;
enum Configure;
enum Wait;
enum Proc;
struct Pierce;
struct Center;
typedef void (*zftype)(); // init
typedef void (*uftype)(struct Center *center); // dma
typedef void (*vftype)(); // safe
typedef void (*yftype)(enum Proc proc, enum Wait wait); // main
typedef int (*xftype)(enum Configure query); // info
typedef void (*wftype)(enum Micro shader, int base, int limit); // draw
typedef int (*rftype)(int size, struct Pierce *data); // ready
typedef void (*sftype)(enum Configure hint); // wake
void planeInit(zftype init, uftype dma, vftype safe, yftype main, xftype info, wftype draw, rftype ready, sftype wake, zftype boot);
void planeAddarg(const char *str);
int planeInfo(enum Configure cfg);
void planeSafe(enum Proc proc, enum Wait wait, enum Configure hint);
void planeMain();
void planeWake(enum Configure hint);
void planeBoot();
void planraWake(enum Configure hint);
void planraBoot();
