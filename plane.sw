import plane
import AppKit
import Metal

func makeLibrary(filepath: String)
{
	print(filepath)
}
func cmdWake(_ idx: CInt)
{
	let cmd = readFile(idx)
	print("cmdWake \(cmd.num!)")
}

// MAIN

var argc = 0
var optc = 3
var lstc = 3
for _ in CommandLine.arguments {argc = argc + 1}
utilAlloc(Int32(argc),Int32(optc),Int32(lstc));
argc = 0; for arg in CommandLine.arguments {utilArg(Int32(argc),arg); argc = argc + 1}
utilEnvInt(Int32(0),"io")


makeLibrary(filepath:"planeG.so")
let cmdpipe = pipeInit("helo", "ok")
callInit(cmdWake, cmdpipe)
