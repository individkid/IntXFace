argxC.o: argx.h face.h type.h
faceC.o: face.h
facerC: faceC.o
facerC.o: face.h
facerHs: face.hs faceC.o
facerLua: face.so
fileC: faceC.o typeC.o
fileC.o: face.h type.h
filerLua: face.so type.lua
holeC: faceC.o typeC.o
holeC.o: face.h type.h
lineC: faceC.o typeC.o
lineC.o: face.h type.h
meticC.o: metic.h
planeC.o: argx.h face.h metic.h plane.h share.h type.h
planeSw: argxC.o faceC.o planeC.o planeG.so shareC.o
planeSw.o: argx.h face.h metic.h plane.h share.h type.h
shareC.o: argx.h face.h share.h type.h
spaceHs: face.hs faceC.o naive.hs type.hs
spacraHs: naive.hs
type.c: type.src
type.h: type.src
type.hs: type.src
type.lua: type.src
type.sw: type.src
typeC.o: face.h type.h
typeSw.o: face.h
typer.c: type.src typra.src
typer.h: type.src typra.src
typer.hs: type.src typra.src
typer.lua: type.src typra.src
typer.sw: type.src typra.src
typerC: faceC.o
typerC.o: face.h typer.h
typerHs: face.hs faceC.o
typerLua: face.so
typerSw: faceC.o
typerSw.o: face.h
typraLua: type.src typra.src
