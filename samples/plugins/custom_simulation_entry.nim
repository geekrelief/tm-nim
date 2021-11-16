import tm
import strformat

type
  SimState* = object of tm_simulation_state_o
    allocator: ptr tm_allocator_i
    count: int

const version = TM_VERSION(0, 1, 0)
var log: ptr tm_logger_api

proc start (args: ptr tm_simulation_start_args_t): ptr tm_simulation_state_o {.cdecl.} =
  result = args.allocator.alloc(SimState(allocator: args.allocator, count: 0))
  log.info("start simulation!")

proc stop (state: ptr tm_simulation_state_o, commands: ptr tm_entity_commands_o) {.cdecl.} =
  let s = cast[ptr SimState](state)
  log.info(&"stop simulation {s.count = }")
  s.allocator.free(s)

proc tick (state: ptr tm_simulation_state_o, args: ptr tm_simulation_frame_args_t) {.cdecl.} =
  let s = cast[ptr SimState](state)
  inc s.count
  log.info(&"tick Count: {s.count}. Frame time: {args.dt:.2}")

var simulationEntryI = tm_simulation_entry_i(
  id: TM_STATIC_HASH("tm_custom_simulation_entry"),
  displayName: "Custom Simulation Entry",
  start: start,
  stop: stop,
  tick: tick)

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  reg.add_or_remove_impl load, simulationEntryI
  reg.get_api_for log
  log.info(&"{version=}: " & (if not load: "Unloading old plugin" else: "Hello from Custom simulation entry!"))