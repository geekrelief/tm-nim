import tm
import strformat

type
  SimState* = object of tmSimulationStateO
    allocator: ptr tmAllocatorI
    count: int

const version = TmVersion(0, 1, 0)
var log: ptr tmLoggerApi

proc start (args: ptr tmSimulationStartArgsT): ptr tmSimulationStateO {.cdecl.} =
  result = tm_alloc(args.allocator, SimState(allocator: args.allocator, count: 0))
  log.info("start simulation!")

proc stop (state: ptr tmSimulationStateO, commands: ptr tmEntityCommandsO) {.cdecl.} =
  let s = cast[ptr SimState](state)
  log.info(&"stop simulation {s.count = }")
  tm_free(s.allocator, s)

proc tick (state: ptr tmSimulationStateO, args: ptr tmSimulationFrameArgsT) {.cdecl.} =
  let s = cast[ptr SimState](state)
  inc s.count
  log.info(&"tick Count: {s.count}. Frame time: {args.dt:.2}")

let simulationEntryI = tmSimulationEntryI(
  id: TmStaticHash("tm_custom_simulation_entry"),
  displayName: "Custom Simulation Entry",
  start: start,
  stop: stop,
  tick: tick)

proc tmLoadPlugin(reg: ptr tmApiRegistryApi, load: bool) {.callback.} =
  if load: 
    NimMain()

  reg.tmAddOrRemoveImpl load, simulationEntryI
  reg.tmGetApiFor log
  log.info(&"{version=}: " & (if not load: "Unloading old plugin" else: "Hello from Custom simulation entry!"))