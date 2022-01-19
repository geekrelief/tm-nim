include tm
import std / strformat
import elvis

var
  log: ptr tm_logger_api

gen animation_state_machine, api_registry, application, entity, error, input, 
  localizer, render_component, transform_component, shader, simulation, 
  ui, tag_component, the_truth_assets, gamestate:
  var `it api`: ptr `tm it api`


type input_state_t = object
  mouse_delta: tm_vec2_t
  held_keys: array[TM_INPUT_KEYBOARD_ITEM_COUNT, bool]
  left_mouse_held: bool
  left_mouse_pressed: bool
  #nim automatically pads use {.packed.} if you need tight packing

type simulation_state_o = object of tm_simulation_state_o
  allocator: ptr tm_allocator_i

  # For interfacing with the `tm_the_truth_api`.
  tt: ptr tm_the_truth_o

  # For interfacing with the `tm_entity_api`.
  entity_ctx: ptr tm_entity_context_o

  # For interfacing with the `tm_simulation_api`.
  simulation_ctx: ptr tm_simulation_o

  # For interfacing with many functions in `the_tm_truth_assets_api`.
  asset_root: tm_tt_id_t

  # Contains keyboard and mouse input state.
  input: input_state_t

  player: tm_entity_t
  player_camera_pivot: tm_entity_t
  checkpoint: tm_entity_t
  checkpoints_positions: array[8, tm_vec3_t]
  processed_events: uint64

  particle_entity: tm_tt_id_t
  rb: ptr tm_renderer_backend_i

  current_checkpoint: uint32
  camera_tilt: float

  # Current score
  score: float

  # For giving some extra time to press jump.
  last_standing_time: float64

  # Component types
  anima_component: tm_component_type_t
  mover_component: tm_component_type_t
  render_component: tm_component_type_t
  tag_component: tm_component_type_t
  transform_component: tm_component_type_t

  # Component managers
  trans_mgr: ptr tm_transform_component_manager_o
  tag_mgr: ptr tm_tag_component_manager_o

  mouse_captured: bool

type simulate_persistent_state = object
  current_checkpoint: uint32
  camera_tilt: float
  score: float
  last_standing_time: float64

proc serialize(s, d: pointer) {.cdecl.} =
  let source = cast[ptr simulation_state_o](s)
  let dest = cast[ptr simulate_persistent_state](d)

  gen current_checkpoint, camera_tilt, score, last_standing_time:
    dest.it = source.it

proc deserialize(d, s: pointer) {.cdecl.} =
  let dest = cast[ptr simulation_state_o](d)
  let source = cast[ptr simulate_persistent_state](s)

  gen current_checkpoint, camera_tilt, score, last_standing_time:
    dest.it = source.it

proc find_root_entity(entity_ctx: ptr tm_entity_context_o, e: tm_entity_t): tm_entity_t =
  result = e
  while true:
    let par = entity_api.asset_parent(entity_ctx, result)
    if par.u64 == 0:
      break
    result = par

proc start(args: ptr tm_simulation_start_args_t): ptr tm_simulation_state_o {.cdecl.} =
  result = args.allocator.alloc(simulation_state_o(
    allocator: args.allocator,
    tt: args.tt,
    entity_ctx: args.entity_ctx,
    simulation_ctx: args.simulation_ctx,
    asset_root: args.asset_root))
  let s = cast[ptr simulation_state_o](result)

  gen (mover, PHYSX_MOVER), (anima, ANIMATION_STATE_MACHINE), (render, RENDER), (tag, TAG), (transform, TRANSFORM):
    s.`it[0] component` = entity_api.lookup_component_type(s.entity_ctx, `TM_TT_TYPE_HASH it[1] COMPONENT`)
  
  gen (trans, transform), (tag, tag):
    s.`it[0] mgr` = cast[ptr `tm it[1] component_manager_o`](entity_api.component_manager(s.entity_ctx, s.`it[1] component`))
  
  gen player, player_camera_pivot, checkpoint:
    s.it = tag_component_api.find_first(s.tag_mgr, TM_STATIC_HASH($$it))
  
  s.camera_tilt = 3.18f
  s.particle_entity = the_truth_assets_api.asset_object_from_path(s.tt, s.asset_root, "vfx/particles.entity")

  let camera = tag_component_api.find_first(s.tag_mgr, TM_STATIC_HASH("camera"))
  simulation_api.set_camera(s.simulation_ctx, camera)

  let root_entity = find_root_entity(s.entity_ctx, s.player)
  for i in 0..<8:
    let checkpoint_path = &"Checkpoints/checkpoint-{i}"
    let c: tm_entity_t = entity_api.resolve_path(s.entity_ctx, root_entity, checkpoint_path.cstring)
    TM_ASSERT(entity_api.is_alive(s.entity_ctx, c), error_api.def, "Failed to find checkpoint entity"):
      continue
    s.checkpoints_positions[i] = transform_component_api.get_position(s.trans_mgr, c)
  
  const singleton_name = "third_person_simulation_state"
  let gamestate = entity_api.gamestate(s.entity_ctx)
  let gs = tm_gamestate_singleton_t(
    name: singleton_name,
    size: sizeof(simulate_persistent_state).uint32,
    serialize: serialize,
    deserialize: deserialize)
  
  gamestate_api.add_singleton(gamestate, gs, s)
  discard gamestate_api.deserialize_singleton(gamestate, singleton_name, s)
  s.rb = first_implementation(tm_global_api_registry, tm_renderer_backend_i)

proc stop(state: ptr tm_simulation_state_o, commands: ptr tm_entity_commands_o) {.cdecl.} =
  var s = cast[ptr simulation_state_o](state)
  s.allocator.free(s)

proc private_set_shader_constant(io: ptr tm_shader_io_o, res_buf: ptr tm_renderer_resource_command_buffer_o, instance: ptr tm_shader_constant_buffer_instance_t, name: tm_strhash_t, data: pointer, data_size: uint32) =
  var 
    constant: tm_shader_constant_t
    constant_offset: uint32
  
  if shader_api.lookup_constant(io, name, addr constant, addr constant_offset):
    let update = tm_shader_constant_update_t(instance_id: instance.instance_id, constant_offset: constant_offset, num_bytes: data_size, data: data)
    shader_api.update_constants(io, res_buf, addr update, 1)

proc private_adjust_effect_start_color(state: ptr tm_simulation_state_o, p: tm_entity_t, color: tm_vec3_t) =
  # Show how to poke at a custom shader variable ('start_color') exposed in a creation graph bound ot a specific draw call ('vfx')
  let s = cast[ptr simulation_state_o](state)
  var rc = cast[ptr tm_render_component_public_t](entity_api.get_component(s.entity_ctx, p, s.render_component))
  let draw: ptr tm_creation_graph_draw_call_data_t = render_component_api.draw_call(rc, TM_STATIC_HASH("vfx"))

  if ?draw.shader:
    var res_buf: ptr tm_renderer_resource_command_buffer_o