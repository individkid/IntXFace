dofile("type.lua")
initialize = hideChange("Change(cfg:Emerg(Planes)vld:Valid(Towrite)idx:Int(0)siz:Int(0))")
finalize = hideChange("Change(cfg:Emerg(Emergs)vld:Valid(Towrite)idx:Int(0)siz:Int(0))")
ident = typeWrap("Spacez","spacerLua","spaceHs")
writeChange(initialize,ident)
writeChange(finalize,ident)
if waitExit() < 0 then io.stderr:write("spacerLua: bad exit status: spaceHs\n"); os.exit(-1) end
