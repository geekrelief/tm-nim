
include tm

import strformat
const 
  version = TM_VERSION(0, 1, 0)
var 
  logger: ptr tm_logger_api
  count = 0

proc start (args: ptr tm_simulation_start_args_t): ptr tm_simulation_state_o {.cdecl.} =
  result = tm_alloc(args[].allocator, tm_simulation_state_o(allocator: args[].allocator))
  logger.print(TM_LOG_TYPE_INFO, $<"start simulation!")

proc stop (state: ptr tm_simulation_state_o, commands: ptr tm_entity_commands_o) {.cdecl.} =
  tm_free(state[].allocator, state)
  logger.print(TM_LOG_TYPE_INFO, $<"stop simulation")

proc tick (state: ptr tm_simulation_state_o, args: ptr tm_simulation_frame_args_t) {.cdecl.} =
  # called once a frame
  inc count
  logger.print(TM_LOG_TYPE_INFO, $< &"!!tock {count=}!")

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

  logger.print(TM_LOG_TYPE_INFO, $<(&"{version=}: " & (if not load: "Unloading old plugin" else: "Hello from Custom simulation!")))