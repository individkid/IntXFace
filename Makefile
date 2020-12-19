#    Makefile
#    Copyright (C) 2019  Paul Coelho
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

.SECONDARY:
all: facer.log typra.log typer.log filer.log spacra.log line plane space trade planra

ifndef DEPEND
# lua depend.lua > depend.mk
include depend.mk
endif

facer.log: facerC facerHs facerLua
	./facerC > facer.log
	./facerHs >> facer.log
	./facerLua >> facer.log
typra.log: typraLua
	./typraLua > typra.log
typer.log: typerC typerHs typerLua typerSw
	./typerC > typer.log
filer.log: filerLua file
	rm -f *.-- .*.-- ..*.-- ...*.--
	./filerLua > filer.log
spacra.log: spacra
	./spacra -xc > spacra.log

%: %C
	ln -f $< $@
%: %Hs
	ln -f $< $@
%: %Lua
	ln -f $< $@
%: %Sw
	ln -f $< $@

LIBRARIES = -llua -lportaudio
%C: %C.o
	clang++ -o $@ $(filter %C.o,$^) ${LIBRARIES}
%Hs: %.hs
	ghc -L/usr/lib -o $@ $< $(filter %C.o,$^) ${LIBRARIES} -v0 2> $*.out
%Lua: %.lua
	echo '#!/usr/bin/env lua' > $@ ; echo 'dofile "'$<'"' >> $@ ; chmod +x $@
%Sw: %Sw.o
	swiftc -o $@ $< $(filter %C.o,$^) -L /usr/local/lib ${LIBRARIES}

%.so: %C.o
	clang -o $@ -fPIC -shared $^ -llua
%.so: %G.o
	xcrun -sdk macosx metallib -o $@ $<

%C.o: %.c
	clang -o $@ -c $< -I /usr/local/include/lua
%C.o: %.m
	clang -o $@ -c $< -I /usr/local/include/lua
%C.o: %.cpp
	clang -o $@ -c $< -I /usr/local/include/lua
%Sw.o: %.sw
	cat $(filter %.sw,$^) | swiftc -o $@ -I . -c -
%G.o: %.metal
	xcrun -sdk macosx metal -O2 -std=macos-metal2.2 -o $@ -c $<

%.metal: %.g
	cp $< $@

%.h: %.gen
	lua $< $@
%.c: %.gen
	lua $< $@
%.hs: %.gen
	lua $< $@
%.lua: %.gen
	lua $< $@
%.sw: %.gen
	lua $< $@
%.g: %.gen
	lua $< $@

.PHONY:
clean:
	rm -f type.h type.c type.hs type.lua type.sw
	rm -f typer.h typer.c typer.hs typer.lua typer.sw
	rm -f typra facer typer filer planra spacra
	rm -f trade file line plane space
	rm -f *C *Hs *Lua *Sw
	rm -f *.err *.out *.log *.tmp
	rm -f *.-- .*.-- ..*.-- ...*.--
	rm -f *.o *.so *.hi *_stub.h a.* *.metal
	rm -f depend type main help

