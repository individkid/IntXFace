datxC.o: datx.c datx.h proto.h type.h
faceC.o: face.c face.h proto.h
faceCpp.o: face.cpp face.h proto.h wrap.h
facer.log: facer.lua facerC facerHs facerLua luax.so
facerC: faceC.o facerC.o protoC.o
facerC.o: face.h facer.c proto.h
facerHs: face.hs faceC.o facer.hs protoC.o
facerLua: facer.lua
fileC: faceC.o fileC.o fileC.typeC.o protoC.o
fileC.o: face.h file.c proto.h type.h
fileC.type.c: luax.so show.lua type.gen
fileC.typeC.o: face.h type.h
filer.log: fileC filer.lua filerLua type.lua
filerLua: filer.lua
fragmentPracticeG: vulkan.g
fragmentComputeG: vulkan.g
hole: holeC
holeC: faceC.o holeC.o holeC.typeC.o protoC.o
holeC.o: face.h hole.c proto.h type.h
holeC.type.c: luax.so show.lua type.gen
holeC.typeC.o: face.h type.h
line: lineCpp
lineCpp: faceC.o lineCpp.o lineCpp.typeC.o protoC.o
lineCpp.o: face.h line.cpp proto.h type.h
lineCpp.type.c: luax.so show.lua type.gen
lineCpp.typeC.o: face.h type.h
luax.so: faceC.o faceCpp.o luaxC.o luaxCpp.o protoC.o wrapCpp.o
luaxC.o: face.h luax.c luax.h proto.h
luaxCpp.o: luax.cpp luax.h proto.h wrap.h
planra.log: planraC
planraC: datxC.o faceC.o planraC.o planraC.typeC.o protoC.o wrapCpp.o
planraC.o: datx.h face.h luax.h metx.h plane.h planra.c proto.h type.h
planraC.type.c: luax.so show.lua type.gen
planraC.typeC.o: face.h type.h
planraCpp: faceC.o planraCpp.o planraCpp.typeC.o protoC.o
planraCpp.o: plane.h planraCpp.mk proto.h type.h vulkan.cpp
planraCpp.type.c: luax.so show.lua type.gen
planraCpp.typeC.o: face.h type.h
protoC.o: proto.c proto.h
share: shareC
shareC: datxC.o faceC.o faceCpp.o luaxC.o luaxCpp.o protoC.o shareC.o shareC.typeC.o wrapCpp.o
shareC.o: datx.h face.h proto.h share.c type.h
shareC.type.c: luax.so show.lua type.gen
shareC.typeC.o: face.h type.h
sharer.log: luax.so shareC sharer.lua sharerLua type.lua
sharerLua: sharer.lua
space: spaceHs
spaceHs: face.hs faceC.o naive.hs protoC.o space.hs spaceHs.type.hs
spaceHs.type.hs: luax.so show.lua type.gen
spacer.log: luax.so spaceHs spacer.lua spacerLua type.lua
spacerLua: spacer.lua
spacra.log: spacraHs
spacraHs: face.hs faceC.o naive.hs protoC.o spacra.hs spacraHs.type.hs
spacraHs.type.hs: luax.so show.lua type.gen
type.h: luax.so show.lua type.gen
type.lua: luax.so show.lua type.gen
typer.c: luax.so show.lua test.lua typer.gen
typer.h: luax.so show.lua test.lua typer.gen
typer.hs: luax.so show.lua test.lua typer.gen
typer.log: typer.lua typerC typerHs typerLua
typer.lua: luax.so show.lua test.lua typer.gen
typerC: faceC.o protoC.o typerC.o
typerC.o: face.h typer.c typer.h
typerHs: face.hs faceC.o protoC.o
typra.log: luax.so show.lua test.lua typra.lua typraLua
typraLua: typra.lua
vertexPracticeG: vulkan.g
vertexComputeG: vulkan.g
wrapCpp.o: proto.h wrap.cpp wrap.h
