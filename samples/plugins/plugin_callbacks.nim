import tm
import strformat

using
  p: ptr tm_plugin_o

const version = TM_VERSION(0, 1, 0)
var log: ptr tm_logger_api

proc init (p; allocator: ptr tm_allocator_i) {.cdecl.} =
  log.info("!! plugin callback: init")

proc shutdown (p) {.cdecl.} =
  log.info(&"!! plugin callback: shutdown")

proc tick (p; dt: cfloat) {.cdecl.} =
  log.info(&"!! plugin tick {dt = :.2}")

var initI = tm_plugin_init_i(init: init)
var shutdownI = tm_plugin_shutdown_i(shutdown: shutdown)
let tickI = tm_plugin_tick_i(tick: tick)

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  reg.get_api_for(log)
  log.info("From plugin_callbacks")

  reg.add_or_remove_impl load, initI, shutdownI #, tickI