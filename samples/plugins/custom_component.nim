import tm
import std / [math]
import strformat

const version = TmVersion(0, 2, 0)

var 
  entityApi: ptr tmEntityApi
  #transformComponentApi: ptr tmTransformComponentApi
  tempAllocatorApi: ptr tmTempAllocatorApi
  truthApi: ptr tmTheTruthApi
  truthCommonTypesApi: ptr tmTheTruthCommonTypesApi
  localizerApi: ptr tmLocalizerApi
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
  localizerApi.TmLocalize("Samples")

let editor_aspect = tmCiEditorUiI(category: componentCategory)

# Use the "tm_type" pragma to map our proc to a TM function typedef (note TM's function typedef is not a function pointer!)
proc truthCreateTypes(tt: ptr tmTheTruthO) {.cdecl, tmType: tmTheTruthCreateTypesI.}  =
  let customComponentProperties = [
    tmTheTruthPropertyDefinitionT(name: "frequency", `type`: TmTheTruthPropertyTypeFloat),
    tmTheTruthPropertyDefinitionT(name: "amplitude", `type`: TmTheTruthPropertyTypeFloat)
  ]

  let 
    customComponentType = truthApi.createObjectType(tt, TtTypeCustomComponent, customComponentProperties[0].unsafeAddr, customComponentProperties.len.uint32)
    defaultObject = truthApi.quickCreateObject(tt, TmTtNoUndoScope, TtTypeHashCustomComponent, Frequency, 2.0, Amplitude, 1.0, -1)
  truthApi.set_default_object(tt, customComponentType, defaultObject)
  truthApi.set_aspect(tt, customComponentType, TmCiEditorUi, editor_aspect.unsafeAddr)


proc componentLoadAsset(manager: ptr tmComponentManagerO, commands: ptr tmEntityCommandsO, e: tmEntityT, compData: pointer, tt: ptr tmTheTruthO, compAsset: tmTtIdT): bool {.cdecl.} =
  var 
    c = cast[ptr CustomComponentT](compData)
    compAssetR: ptr tmTheTruthObjectO = truthApi.read(tt, compAsset)
    ownerAsset: tmTtIdT = truthApi.owner(tt, compAsset)

  #[
  let trAsset = truthApi.find_subobject_of_type(tt, 
    truthApi.read(tt, ownerAsset), 
    TM_TT_PROP_ENTITY_COMPONENTS.uint32, 
    truthApi.object_type_from_name_hash(tt, TM_TT_TYPE_HASH_TRANSFORM_COMPONENT))
  let pos = truthCommonTypesApi.get_position(tt, truthApi.read(tt, trAsset), TM_TT_PROP_TRANSFORM_COMPONENT_LOCAL_POSITION.uint32)
  ]#

  let y = truthApi.quick_get_property(tt, ownerAsset, TM_TT_PROP_ENTITY_COMPONENTS, 0, TM_TT_PROP_TRANSFORM_COMPONENT_LOCAL_POSITION, 1, -1).f32
  c.y0 = y
  
  c.frequency = truthApi.getFloat(tt, compAssetR, Frequency.uint32)
  c.amplitude = truthApi.getFloat(tt, compAssetR, Amplitude.uint32)
  true

proc componentAssetReloaded(man: ptr tmComponentManagerO, commands: ptr tmEntityCommandsO, e: tmEntityT, data: pointer) {.cdecl.} =
  log.info("componentAssetReloaed")

proc componentCreate(ctx: ptr tmEntityContextO) {.cdecl, tmType: tmEntityCreateComponentI.} =
  let component = tmComponentI(
    name: TtTypeCustomComponent,
    bytes: sizeof(CustomComponentT).uint32,
    loadAsset: componentLoadAsset,
    assetReloaded: componentAssetReloaded,
    )
  discard entityApi.registerComponent(ctx, component.unsafeAddr)

proc engineUpdateCustom(inst: ptr tmEngineO, data: ptr tmEngineUpdateSetT, commands: ptr tmEntityCommandsO) {.cdecl.} =
  var 
    ta = tempAllocatorApi.init()
    modTransform: ptr tmEntityT
    ctx = cast[ptr tmEntityContextO](inst)
    t = 0'f64
 
  for bb in items(data.blackboardStart, data.blackboardEnd):
    if bb.id == TmEntityBbTime: 
      t = bb.doubleValue

  for a in items(data.arrays, data.numArrays):
    var
      custom = cast[ptr CustomComponentT](a.components[0])
      transform = cast[ptr tmTransformComponentT](a.components[1])
    for _, c, tr, e in mrows(custom, transform, a.entities, a.n):
      let y = c.y0 + c.amplitude * sin(float(t) * c.frequency)
      tr.world.pos.x = y
      tr.world.pos.y = sin(float(t) * 30.5 )*0.08323f + c.y0
      tr.world.pos.z = cos(float(t) * 20.712 )*0.07463 + tr.world.pos.y # + 0.5
      let angle = t * c.frequency
      tr.world.rot = tmQuaternionFromEuler(vec3(x = angle * 0.981, y = angle * 1.23, z = angle))
      inc tr.version
      discard tmCarrayTempPush(modTransform, e, ta)
  
  entityApi.notify(ctx, data.engine.components[1], modTransform, tmCarraySize(modTransform).uint32)


proc engineFilterCustom(inst: ptr tmEngineO, components: ptr tmComponentTypeT, numComponents: uint32, mask: ptr tmComponentMaskT): bool {.cdecl.} =
  result = tmEntityMaskHasComponent(mask, components[0]) and tmEntityMaskHasComponent(mask, components[1])

proc componentRegisterEngine(ctx: ptr tmEntityContextO) {.tmType: tmEntityRegisterEnginesSimulationI} =
  let
    customComponent = entityApi.lookupComponentType(ctx, TtTypeHashCustomComponent)
    transformComponent = entityApi.lookupComponentType(ctx, TmTtTypeHashTransformComponent)
  
  var e = initEngineI(
    uiName = "Custom Component",
    hash = TmStaticHash("TM_ENGINE__CUSTOM_COMPONENT"),
    components = [customComponent, transformComponent],
    writes = [false, true],

    update = engineUpdateCustom,
    filter = engineFilterCustom,
    inst = ctx,
  )
  entityApi.registerEngine(ctx, e.unsafeAddr)

proc tmLoadPlugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  reg.tmGetApiFor entityApi, #[transformComponentApi,]# tempAllocatorApi, truthApi, 
    truthCommonTypesApi, localizerApi, log

  if load:
    log.info(&"custom component {version}")

  reg.tmAddOrRemoveImpl load, truthCreateTypes, componentCreate, componentRegisterEngine