all: facer.log typra.log typer.log filer.log

facer.log: facerC facerHs facerLua
	./facerC > facer.log
	./facerHs >> facer.log
	./facerLua >> facer.log

typra.log: typra
	./typra > typra.log

typer.log: typerC typerHs typerLua
	./typerC > typer.log

filer.log: filer
	./filer > filer.log

%C: %.c faceC.o
	clang -o $@ $(filter-out %.h,$^) -llua
typerC: typer.h
fileC lineC planeC: typeC.o type.h
%: %C
	ln -f $< $@

%Hs: %.hs face.hs faceC.o
	ghc -o $@ $< faceC.o -llua -v0 2> $*.out
spaceHs: type.hs
%: %Hs
	ln -f $< $@

wrap = echo '\#!/usr/bin/env lua' > $@ ; echo 'dofile "'$<'"' >> $@ ; chmod +x $@
%Gen: %.gen
	$(wrap)
%Lua: %.lua
	$(wrap)
%: %Lua
	ln -f $< $@
facerLua: face.so
typraLua typerGen: show.lua test.lua
typerLua: face.so typer.lua
typeGen: show.lua
filerLua: face.so type.lua file
linerLua playLua ballLua: face.so type.lua file line
planerLua sculptLua playLua ballLua: face.so type.lua file plane
spacerLua sculptLua printLua playLua ballLua: face.so type.lua file space

%.so: %C.o
	clang -o $@ -fPIC -shared $^ -llua
type.so: faceC.o

%C.o: %.c %.h
	clang -o $@ -c $< -I /usr/local/include/lua

%.h: %Gen
	./$< $@
%.c: %Gen
	./$< $@
%.hs: %Gen
	./$< $@
%.lua: %Gen
	./$< $@

.PHONY: clean
clean:
	rm -f type.h type.c type.hs type.lua
	rm -f typer.h typer.c typer.hs typer.lua
	rm -f typra filer file line plane space 
	rm -f *C *Hs *Lua *Gen *.out *.log
	rm -f *.o *.so *.hi *_stub.h *.txt
