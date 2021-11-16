import tm
import std / [math]
import strformat

const version = TmVersion(0, 2, 0)

var 
  entityApi: ptr tm_entity_api
  tempAllocatorApi: ptr tm_temp_allocator_api
  truthApi: ptr tm_the_truth_api
  truthCommonTypesApi: ptr tm_the_truth_common_types_api
  localizerApi: ptr tm_localizer_api
  log: ptr tm_logger_api

const
  TtTypeCustomComponent = "custom_component"
  TtTypeHashCustomComponent = TM_STATIC_HASH(TtTypeCustomComponent)

type
  CustomComponentE = enum
    Frequency
    Amplitude

  CustomComponentT = object
    y0, frequency, amplitude: float
  
proc componentCategory(): cstring {.cdecl.} =
  localizerApi.TM_LOCALIZE("Samples")

var editor_aspect = tm_ci_editor_ui_i(category: componentCategory)

# Use the "tm_type" pragma to map our proc to a TM function typedef (note TM's function typedef is not a function pointer!)
proc truthCreateTypes(tt: ptr tm_the_truth_o) {.cdecl, tmType: tm_the_truth_create_types_i.}  =
  var customComponentProperties = [
    tmTheTruthPropertyDefinitionT(name: "frequency", `type`: TM_THE_TRUTH_PROPERTY_TYPE_FLOAT),
    tmTheTruthPropertyDefinitionT(name: "amplitude", `type`: TM_THE_TRUTH_PROPERTY_TYPE_FLOAT)
  ]

  let 
    customComponentType = truthApi.create_object_type(tt, TtTypeCustomComponent, customComponentProperties[0].addr, customComponentProperties.len.uint32)
    defaultObject = truthApi.quick_create_object(tt, TM_TT_NO_UNDO_SCOPE, TtTypeHashCustomComponent, Frequency, 2.0, Amplitude, 1.0, -1)
  truthApi.set_default_object(tt, customComponentType, defaultObject)
  truthApi.set_aspect(tt, customComponentType, TM_CI_EDITOR_UI, editor_aspect.addr)


proc componentLoadAsset(manager: ptr tm_component_manager_o, commands: ptr tm_entity_commands_o, e: tm_entity_t, compData: pointer, tt: ptr tm_the_truth_o, compAsset: tm_tt_id_t): bool {.cdecl.} =
  var 
    c = cast[ptr CustomComponentT](compData)
    compAssetR: ptr tm_the_truth_object_o = truthApi.read(tt, compAsset)
    ownerAsset: tm_tt_id_t = truthApi.owner(tt, compAsset)

  #[
  let trAsset = truthApi.find_subobject_of_type(tt, 
    truthApi.read(tt, ownerAsset), 
    TM_TT_PROP_ENTITY_COMPONENTS.uint32, 
    truthApi.object_type_from_name_hash(tt, TM_TT_TYPE_HASH_TRANSFORM_COMPONENT))
  let pos = truthCommonTypesApi.get_position(tt, truthApi.read(tt, trAsset), TM_TT_PROP_TRANSFORM_COMPONENT_LOCAL_POSITION.uint32)
  ]#

  let y = truthApi.quick_get_property(tt, ownerAsset, TM_TT_PROP_ENTITY_COMPONENTS, 0, TM_TT_PROP_TRANSFORM_COMPONENT_LOCAL_POSITION, 1, -1).f32
  c.y0 = y
  
  c.frequency = truthApi.get_float(tt, compAssetR, Frequency.uint32)
  c.amplitude = truthApi.get_float(tt, compAssetR, Amplitude.uint32)
  true

proc componentAssetReloaded(man: ptr tm_component_manager_o, commands: ptr tm_entity_commands_o, e: tm_entity_t, data: pointer) {.cdecl.} =
  log.info("componentAssetReloaed")

proc componentCreate(ctx: ptr tm_entity_context_o) {.cdecl, tmType: tm_entity_create_component_i.} =
  var component = tm_component_i(
    name: TtTypeCustomComponent,
    bytes: sizeof(CustomComponentT).uint32,
    loadAsset: componentLoadAsset,
    assetReloaded: componentAssetReloaded,
    )
  discard entityApi.register_component(ctx, component.addr)

proc engineUpdateCustom(inst: ptr tm_engine_o, data: ptr tm_engine_update_set_t, commands: ptr tm_entity_commands_o) {.cdecl.} =
  var 
    ta = tempAllocatorApi.init()
    modTransform: ptr tm_entity_t
    ctx = cast[ptr tm_entity_context_o](inst)
    t = 0'f64
 
  for bb in items(data.blackboardStart, data.blackboardEnd):
    if bb.id == TM_ENTITY_BB_TIME: 
      t = bb.doubleValue

  for a in items(data.arrays, data.numArrays):
    var
      custom = cast[ptr CustomComponentT](a.components[0])
      transform = cast[ptr tm_transform_component_t](a.components[1])
    for _, c, tr, e in mrows(custom, transform, a.entities, a.n):
      let y = c.y0 + c.amplitude * sin(float(t) * c.frequency)
      tr.world.pos.x = y
      tr.world.pos.y = sin(float(t) * 30.5 )*0.08323f + c.y0
      tr.world.pos.z = cos(float(t) * 20.712 )*0.07463 + tr.world.pos.y # + 0.5
      let angle = t * c.frequency
      tr.world.rot = tm_quaternion_from_euler(vec3(x = angle * 0.981, y = angle * 1.23, z = angle))
      inc tr.version
      discard tm_carray_temp_push(modTransform, e, ta)
  
  entityApi.notify(ctx, data.engine.components[1], modTransform, tm_carray_size(modTransform).uint32)


proc engineFilterCustom(inst: ptr tm_engine_o, components: ptr tm_component_type_t, numComponents: uint32, mask: ptr tm_component_mask_t): bool {.cdecl.} =
  result = tm_entity_mask_has_component(mask, components[0]) and tm_entity_mask_has_component(mask, components[1])

proc componentRegisterEngine(ctx: ptr tm_entity_context_o) {.tmType: tm_entity_register_engines_simulation_i} =
  let
    customComponent = entityApi.lookup_component_type(ctx, TtTypeHashCustomComponent)
    transformComponent = entityApi.lookup_component_type(ctx, TM_TT_TYPE_HASH_TRANSFORM_COMPONENT)
  
  var e = initEngineI(
    uiName = "Custom Component",
    hash = TM_STATIC_HASH("TM_ENGINE__CUSTOM_COMPONENT"),
    components = [customComponent, transformComponent],
    writes = [false, true],

    update = engineUpdateCustom,
    filter = engineFilterCustom,
    inst = ctx,
  )
  entityApi.register_engine(ctx, e.addr)

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  reg.get_api_for entityApi, tempAllocatorApi, truthApi, truthCommonTypesApi, localizerApi, log

  if load:
    log.info(&"custom component {version}")

  reg.add_or_remove_impl load, truthCreateTypes, componentCreate, componentRegisterEngine