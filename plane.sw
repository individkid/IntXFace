import argx
import share
import type
import plane
import face
import AppKit
import Metal

var device:MTLDevice!
var layer:CAMetalLayer!
var view:NSView!
var window:NSWindow!
var queue:MTLCommandQueue!
var render:MTLRenderPipelineState!
var param:MTLRenderPassDescriptor!
var depth:MTLDepthStencilState!
var compute:MTLComputePipelineState!
var threads:MTLSize!

var lock = [Refer]()
let event = getEvent()

var numeric = Pend<type.Numeric>()

class Refer
{
	var lock:Int = 0
}
func toMutable<T>(_ list:[T], _ fnc:(_:Int,_:UnsafeMutablePointer<T>)->Void)
{
	let ptr = UnsafeMutablePointer<T>.allocate(capacity:list.count)
	for (val,idx) in zip(list,Swift.Array(0..<list.count)) {ptr[idx] = val}
	fnc(list.count,ptr)
	ptr.deallocate()
}
class Pend<T>
{
	var pend:MTLBuffer!
	var last:MTLBuffer!
	var refer:Refer!
	func set(_ ptr: UnsafeRawPointer, _ range: Range<Int>)
	{
		let len:Int = range.upperBound
		let unit:Int = MemoryLayout<T>.size
		let length:Int = len+(unit-len%unit)%unit;
		if (pend == nil && last != nil && last.length < length) {
			pend = device.makeBuffer(length:length)
			pend.contents().copyMemory(from:last.contents(),byteCount:last.length)
			last = nil
		}
		if (pend == nil && last != nil && refer.lock == 0)
		{
			pend = last
			last = nil
		}
		if (pend == nil && last != nil) {
			pend = device.makeBuffer(bytes:last.contents(),length:last.length)
			last = nil
		}
		if (pend == nil) {
			pend = device.makeBuffer(length:length)
		}
		if (pend != nil) {
			let base:Int = range.lowerBound
			let size:Int = range.upperBound-range.lowerBound
			pend.contents().advanced(by:base).copyMemory(from:ptr,byteCount:size)
		}
	}
	func set(_ vals: [T], _ index: Int)
	{
		let siz = MemoryLayout<T>.size
		let base = siz*index
		let limit = base+siz*vals.count
		toMutable(vals) {(len,ptr) in
		set(UnsafePointer<T>(ptr),base..<limit)}
	}
	func set(_ val: [T]?, _ index: Int)
	{
		guard let vals = val else {return}
		set(vals,index)
	}
	func set<S>(_ val: S, _ field: PartialKeyPath<T>)
	{
		guard let fld = MemoryLayout<T>.offset(of:field) else {exitErr(#file,#line,-1);return}
		let siz = MemoryLayout<S>.size
		toMutable([val]) {(len,ptr) in
		set(UnsafePointer<S>(ptr),fld..<fld+siz)}
	}
	func get() -> MTLBuffer
	{
		if (last == nil && pend != nil) {
			refer = Refer()
			last = pend
			pend = nil
		}
		if (last != nil) {
			lock.append(refer)
			refer.lock += 1
		}
		return last
	}
}
func getRect() -> NSRect
{
	if let temp = window {
		return temp.contentRect(forFrameRect:temp.frame)
	} else {
		return NSMakeRect(0.0, 0.0, 
			CGFloat(planeConfig(plane.WindowWide)),
			CGFloat(planeConfig(plane.WindowHigh)))
	}
}
func getPoint() -> NSPoint
{
	var point = NSEvent.mouseLocation
	let frame:CGRect = window.frame
	point.x = point.x - NSMinX(frame)
	point.y = point.y - NSMinY(frame)
	return point
}
func getTexture(_ rect:NSRect) -> MTLTexture?
{
	let text = MTLTextureDescriptor()
	text.height = Int(rect.height)
	text.width = Int(rect.width)
	text.pixelFormat = .depth32Float
	text.storageMode = .private
	return device.makeTexture(descriptor:text)
}
class getEvent : NSObject, NSWindowDelegate
{
	func windowShouldClose(_ sender: NSWindow) -> Bool
	{
		// loopStop()
		return true
	}
	func windowDidResize(_ notification: Notification)
	{
		// swiftSize()
	}
}
func setEvent(_ type:NSEvent.EventTypeMask, _ handler: @escaping (_:NSEvent) -> NSEvent?)
{
	NSEvent.addLocalMonitorForEvents(matching:type,handler:handler)
}
func swiftInit()
{
	device = MTLCreateSystemDefaultDevice()
	let rect = NSMakeRect(
		CGFloat(planeConfig(plane.PictureMinX)), CGFloat(planeConfig(plane.PictureMinY)),
		CGFloat(planeConfig(plane.PictureWide)), CGFloat(planeConfig(plane.PictureHigh)))
	layer = CAMetalLayer()
	layer.device = device
	layer.pixelFormat = .bgra8Unorm
	layer.framebufferOnly = true
	layer.frame = rect
	view = NSView(frame:rect)
	view.wantsLayer = true
	view.layer = layer
	let mask:NSWindow.StyleMask = [.titled, .closable, .miniaturizable, .resizable]
	window = NSWindow(contentRect: rect, styleMask: mask, backing: .buffered, defer: true)
	window.title = "plane"
	window.makeKeyAndOrderFront(nil)
	window.contentView = view
	window.delegate = event
	queue = device.makeCommandQueue()
	guard let library:MTLLibrary = try? device.makeLibrary(filepath:"planeG.so") else {
		print("cannot make library");exitErr(#file,#line,-1);return}
	guard let vertex_render = library.makeFunction(name:"vertex_render") else {
		print("cannot make vertex_render");exitErr(#file,#line,-1);return}
	guard let fragment_render = library.makeFunction(name:"fragment_render") else {
		print("cannot make fragment_render");exitErr(#file,#line,-1);return}
	guard let kernel_pierce = library.makeFunction(name:"kernel_pierce") else {
		print("cannot make kernel_pierce");exitErr(#file,#line,-1);return}
	let pipe = MTLRenderPipelineDescriptor()
	pipe.vertexFunction = vertex_render
	pipe.fragmentFunction = fragment_render
	pipe.colorAttachments[0].pixelFormat = .bgra8Unorm
	pipe.depthAttachmentPixelFormat = .depth32Float
	render = try? device.makeRenderPipelineState(descriptor:pipe)
	let color = MTLClearColor(red: 0.0, green: 104.0/255.0, blue: 55.0/255.0, alpha: 1.0)
	param = MTLRenderPassDescriptor()
	param.colorAttachments[0].clearColor = color
	param.colorAttachments[0].storeAction = .store
	param.depthAttachment.clearDepth = 0.0 // clip xy -1 to 1; z 0 to 1
	param.depthAttachment.storeAction = .dontCare
	param.depthAttachment.texture = getTexture(rect)
    let desc = MTLDepthStencilDescriptor()
    desc.depthCompareFunction = .greater // left hand rule; z thumb to observer
    desc.isDepthWriteEnabled = true
    depth = device.makeDepthStencilState(descriptor: desc)
	compute = try? device.makeComputePipelineState(function:kernel_pierce)
    threads = device.maxThreadsPerThreadgroup
	// setEvent(.keyDown,swiftKey)
	// setEvent(.leftMouseUp,swiftLeft)
	// setEvent(.rightMouseUp,swiftRight)
	// setEvent(.leftMouseDown,swiftActive)
	// setEvent(.rightMouseDown,swiftActive)
	// setEvent(.mouseMoved,swiftMove)
	// setEvent(.scrollWheel,swiftRoll)
	// setEvent(.applicationDefined,swiftCheck)
}
func swiftMemory()
{
	let client = planeClient()!.pointee
	let siz = Int(client.siz)
	let idx = Int(client.idx)
	switch (client.mem) {
	case (Numerics): numeric.set(Swift.Array(0..<siz).map() {(sub) in client.num![sub]},idx)
	default: exitErr(#file,#line,-1)}
}
func swiftPierce()
{
}
func swiftDisplay()
{
}

// MAIN

shareInit()
planeInit(swiftMemory,swiftPierce,swiftDisplay)
for arg in CommandLine.arguments {useArgument(arg)}
swiftInit()
runProgram()
