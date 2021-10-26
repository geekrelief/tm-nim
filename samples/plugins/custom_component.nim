import tm
import std / [math]
import strformat

const version = TmVersion(0, 1, 0)

var 
  entity: ptr tmEntityApi
  transformComponent: ptr tmTransformComponentApi
  tempAllocator: ptr tmTempAllocatorApi
  truth: ptr tmTheTruthApi
  localizer: ptr tmLocalizerApi
  log: ptr tmLoggerApi

const
  TtTypeCustomComponent = "custom_component"
  TtTypeHashCustomComponent = TmStaticHash(TtTypeCustomComponent)

type
  CustomComponentE = enum
    Frequency
    Amplitude

  CustomComponentT = object
    y0, frequency, amplitude: float

proc componentCategory(): cstring {.cdecl.} =
  localizer.TmLocalize("Samples")

let editor_aspect = tmCiEditorUiI(category: componentCategory)

# Use the "tm_type" pragma to map our proc to a TM function typedef (note TM's function typedef is not a function pointer!)
proc truthCreateTypes(tt: ptr tmTheTruthO) {.cdecl, tmType: tmTheTruthCreateTypesI.}  =
  let customComponentProperties = [
    tmTheTruthPropertyDefinitionT(name: "frequency", `type`: TmTheTruthPropertyTypeFloat),
    tmTheTruthPropertyDefinitionT(name: "amplitude", `type`: TmTheTruthPropertyTypeFloat)
  ]

  let 
    customComponentType = truth.createObjectType(tt, TtTypeCustomComponent, customComponentProperties[0].unsafeAddr, customComponentProperties.len.uint32)
    defaultObject = truth.quickCreateObject(tt, TmTtNoUndoScope, TtTypeHashCustomComponent,
      Frequency, 1.0, Amplitude, 1.0, -1)
  truth.set_default_object(tt, customComponentType, defaultObject)
  truth.set_aspect(tt, customComponentType, TmCiEditorUi, editor_aspect.unsafeAddr)


proc componentLoadAsset(man: ptr tmComponentManagerO, commands: ptr tmEntityCommandsO, e: tmEntityT, cVp: pointer, tt: ptr tmTheTruthO, asset: tmTtIdT): bool {.cdecl.} =
  var 
    c = cast[ptr CustomComponentT](cVp)
    assetR: ptr tmTheTruthObjectO = truth.read(tt, asset)
  c.y0 = 0
  c.frequency = truth.getFloat(tt, assetR, Frequency.uint32)
  c.amplitude = truth.getFloat(tt, assetR, Amplitude.uint32)
  true

proc componentCreate(ctx: ptr tmEntityContextO) {.cdecl, tmType: tmEntityCreateComponentI.} =
  let component = tmComponentI(
    name: TtTypeCustomComponent,
    bytes: sizeof(CustomComponentT).uint32,
    load_asset: componentLoadAsset)
  discard entity.registerComponent(ctx, component.unsafeAddr)

proc engineUpdateCustom(inst: ptr tmEngineO, data: ptr tmEngineUpdateSetT, commands: ptr tmEntityCommandsO) {.cdecl.} =
  var 
    ta = tempAllocator.initTempAllocator()
    modTransform: ptr tmEntityT
    ctx = cast[ptr tmEntityContextO](inst)
    t = 0'f64

  var bb: ptr tmEntityBlackboardValueT = data.blackboardStart
  while bb != data.blackboardEnd:
    if bb.id == TmEntityBbTime: t = bb.doubleValue
    bb += 1

  var a: ptr UncheckedArray[tmEngineUpdateArrayT] = data.arrays.addr
  for j in 0..<data.numArrays:
    var
      custom = cast[ptr CustomComponentT](a[j].components[0])
      transform = cast[ptr tmTransformComponentT](a[j].components[1])
    for i in 0..<a[j].n:
      if custom[i.int].y0 == 0.0:
        custom[i.int].y0 = transform[i.int].world.pos.y
      let y = custom[i.int].y0 + custom[i.int].amplitude * sin(float(t) * custom[i.int].frequency)
      transform[i.int].world.pos.y = y
      inc transform[i.int].version
      discard tmCarrayTempPush(modTransform, a[j].entities[i.int], ta)
  
  entity.notify(ctx, data.engine.components[1], modTransform, tmCarraySize(modTransform).uint32)


proc engineFilterCustom(inst: ptr tmEngineO, components: ptr tmComponentTypeT, numComponents: uint32, mask: ptr tmComponentMaskT): bool {.cdecl.} =
  result = tmEntityMaskHasComponent(mask, components[0]) and tmEntityMaskHasComponent(mask, components[1])

proc componentRegisterEngine(ctx: ptr tmEntityContextO) {.tmType: tmEntityRegisterEnginesSimulationI} =
  let
    customComponent = entity.lookupComponentType(ctx, TtTypeHashCustomComponent)
    transformComponent = entity.lookupComponentType(ctx, TmTtTypeHashTransformComponent)
  
  var e = initEngineI(
    uiName = "Custom Component",
    hash = TmStaticHash("TM_ENGINE__CUSTOM_COMPONENT"),
    components = [customComponent, transformComponent],
    writes = [false, true],

    update = engineUpdateCustom,
    filter = engineFilterCustom,
    inst = ctx,
  )
  entity.registerEngine(ctx, e.unsafeAddr)

proc tmLoadPlugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  reg.tmGetApiFor entity, transformComponent, tempAllocator, truth, localizer, log

  if load:
    log.info("Hi from custom component!")

  reg.tmAddOrRemoveImpl load, truthCreateTypes, componentCreate, componentRegisterEngine