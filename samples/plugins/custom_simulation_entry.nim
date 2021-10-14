include tm

type
  SimState* = object of tm_simulation_state_o
    allocator: ptr tm_allocator_i
    count: int

import strformat
const 
  version = TM_VERSION(0, 1, 0)
var 
  logger: ptr tm_logger_api

proc start (args: ptr tm_simulation_start_args_t): ptr tm_simulation_state_o {.cdecl.} =
  result = tm_alloc(args[].allocator, SimState(allocator: args[].allocator, count: 0))
  logger.info("start simulation!")

proc stop (state: ptr tm_simulation_state_o, commands: ptr tm_entity_commands_o) {.cdecl.} =
  let s = cast[ptr SimState](state)
  logger.info(&"stop simulation {s[].count = }")
  tm_free(s[].allocator, s)

proc tick (state: ptr tm_simulation_state_o, args: ptr tm_simulation_frame_args_t) {.cdecl.} =
  let s = cast[ptr SimState](state)
  inc s[].count
  logger.info(&"tick {s[].count}!")

var simulation_entry_i = tm_simulation_entry_i(
  id: TM_STATIC_HASH("tm_custom_simulation_entry"),
  display_name: "Custom Simulation Entry",
  start: start,
  stop: stop,
  tick: tick)

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  tm_add_or_remove_implementation(reg, load, tm_simulation_entry_i, simulation_entry_i.addr)

  logger = reg.tm_get_api(tm_logger_api)
  logger.info(&"{version=}: " & (if not load: "Unloading old plugin" else: "Hello from Custom simulation!"))