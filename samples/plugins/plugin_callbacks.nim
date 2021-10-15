include tm
import strformat

const version = TM_VERSION(0, 1, 0)
var log: ptr tm_logger_api

proc init (inst: ptr tm_plugin_o, allocator: ptr tm_allocator_i) {.cdecl.} =
  log.info("!! plugin callback: init")

proc shutdown (inst: ptr tm_plugin_o) {.cdecl.} =
  log.info(&"!! plugin callback: shutdown")

proc tick (inst: ptr tm_plugin_o, dt: cfloat) {.cdecl.} =
  log.info(&"!! plugin tick {dt = }")

var plugin_init_i = tm_plugin_init_i(init: init)
var plugin_shutdown_i = tm_plugin_shutdown_i(shutdown: shutdown)
var plugin_tick_i = tm_plugin_tick_i(tick: tick)

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  reg.tm_get_api_for(log)
  log.info("From plugin_callbacks")

  reg.tm_add_or_remove_impl load, 
    plugin_init_i, plugin_shutdown_i #, plugin_tick_i