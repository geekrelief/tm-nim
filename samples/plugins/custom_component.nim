include tm

const version = TM_VERSION(0, 1, 0)

var 
  entity: ptr tm_entity_api
  transformComponent: ptr tm_transform_component_api
  tempAllocator: ptr tm_temp_allocator_api
  truth: ptr tm_the_truth_api
  localizer: ptr tm_localizer_api
  log: ptr tm_logger_api

const
  ttTypeCustomComponent = "tm_custom_component"
  ttTypeHashCustomComponent = TM_STATIC_HASH(ttTypeCustomComponent)

type
  CustomComponentE = enum
    Frequency
    Amplitude

  CustomComponentT = object
    y0, frequency, amplitude: float

proc componentCategory(): cstring {.cdecl.} =
  localizer.TM_LOCALIZE("Samples")

let editor_aspect = tm_ci_editor_ui_i(category: componentCategory)

# Use the "tm_type" pragma to map our proc to a TM function typedef (note TM's function typedef is not a function pointer!)
proc truthCreateTypes(tt: ptr tm_the_truth_o) {.cdecl, tm_type: tm_the_truth_create_types_i.}  =
  let customComponentProperties = [
    tm_the_truth_property_definition_t(name: "frequency", `type`: TM_THE_TRUTH_PROPERTY_TYPE_FLOAT),
    tm_the_truth_property_definition_t(name: "amplitude", `type`: TM_THE_TRUTH_PROPERTY_TYPE_FLOAT)
  ]

  let 
    customComponentType = truth.create_object_type(tt, ttTypeCustomComponent, customComponentProperties[0].unsafeAddr, customComponentProperties.len.uint32)
    defaultObject = truth.quick_create_object(tt, TM_TT_NO_UNDO_SCOPE, ttTypeHashCustomComponent,
      Frequency, 1.0, Amplitude, 1.0, -1)
  truth.set_default_object(tt, customComponentType, defaultObject)
  truth.set_aspect(tt, customComponentType, TM_CI_EDITOR_UI, editor_aspect.unsafeAddr)


proc componentLoadAsset(man: ptr tm_component_manager_o, commands: ptr tm_entity_commands_o, e: tm_entity_t, c_vp: pointer, tt: ptr tm_the_truth_o, asset: tm_tt_id_t): bool {.cdecl.} =
  var 
    c = cast[ptr CustomComponentT](c_vp)
    assetR: ptr tm_the_truth_object_o = truth.read(tt, asset)
  c.y0 = 0
  c.frequency = truth.get_float(tt, assetR, Frequency.uint32)
  c.amplitude = truth.get_float(tt, assetR, Amplitude.uint32)
  true

proc componentCreate(ctx: ptr tm_entity_context_o) {.cdecl.} =
  let component = tm_component_i(
    name: ttTypeCustomComponent,
    bytes: sizeof(CustomComponentT).uint32,
    load_asset: componentLoadAsset)
  discard entity.register_component(ctx, component.unsafeAddr)


proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  reg.tm_get_api_for entity, transformComponent, tempAllocator, truth, localizer, log

  reg.tm_add_or_remove_impl load, truthCreateTypes