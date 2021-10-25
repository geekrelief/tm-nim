import tm
import strformat

using
  p: ptr tmPluginO

const version = TmVersion(0, 1, 0)
var log: ptr tmLoggerApi

proc init (p; allocator: ptr tmAllocatorI) {.cdecl.} =
  log.info("!! plugin callback: init")

proc shutdown (p) {.cdecl.} =
  log.info(&"!! plugin callback: shutdown")

proc tick (p; dt: cfloat) {.cdecl.} =
  log.info(&"!! plugin tick {dt = :.2}")

var initI = tmPluginInitI(init: init)
var shutdownI = tmPluginShutdownI(shutdown: shutdown)
let tickI = tmPluginTickI(tick: tick)

proc tm_load_plugin(reg: ptr tmApiRegistryApi, load: bool) {.callback.} =
  if load: 
    NimMain()

  reg.tmGetApiFor(log)
  log.info("From plugin_callbacks")

  reg.tmAddOrRemoveImpl load, initI, shutdownI #, tickI