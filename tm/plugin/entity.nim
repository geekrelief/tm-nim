#proc initEntitySystemI(
#):tmEntitySystemI

proc initEngineI*(
  # engine system common
  uiName: string,
  hash: tmStrhashT,
  disabled: bool = false,
  exclusive: bool = false,
  components: openArray[tmComponentTypeT],
  writes: openArray[bool],
  beforeMe: openArray[tmStrhashT] = [],
  afterMe: openArray[tmStrhashT] = [],
  phase: tm_strhash_t = tmStrhashT(0),

  # engine
  inst: pointer,
  update: proc (inst: ptr tmEngineO; data: ptr tmEngineUpdateSetT; commands: ptr tmEntityCommandsO) {.cdecl.},
  excluded: openArray[tmComponentTypeT] = [],
  filter: proc (inst: ptr tmEngineO; components: ptr tmComponentTypeT; numComponents: uint32; mask: ptr tmComponentMaskT): bool {.cdecl.}
): tmEngineI =
  tmEngineI(
    uiName: uiName,
    hash: hash,
    disabled: disabled,
    exclusive: exclusive,
    numComponents: components.len.uint32,
    components: toArray(TmMaxComponentsForEngine, components),
    writes: toArray(TmMaxComponentsForEngine, writes),
    beforeMe: toArray(TmMaxDependenciesForEngine, beforeMe),
    afterMe: toArray(TmMaxDependenciesForEngine, afterMe),
    phase: phase,
    
    inst: cast[ptr tmEngineO](inst),
    update: update,
    numExcluded:  excluded.len.uint32,
    excluded: toArray(TmMaxComponentsForEngine, excluded),
    filter: filter
    )