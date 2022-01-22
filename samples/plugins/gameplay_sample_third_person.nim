include tm
import std / [math]

var
  log: ptr tm_logger_api

gen animation_state_machine, api_registry, application, entity, error, input, 
  localizer, render_component, transform_component, shader, simulation, 
  ui, tag_component, the_truth_assets, gamestate:
  var `it api`: ptr `tm it api`


type input_state_t = object
  mouse_delta: tm_vec2_t
  held_keys: array[TM_INPUT_KEYBOARD_ITEM_COUNT.uint64, bool]
  left_mouse_held: bool
  left_mouse_pressed: bool
  #nim automatically pads. Use {.packed.} if you need tight packing

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
  camera_pivot: tm_entity_t
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
    if !par.u64:
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
  
  gen player, camera_pivot, checkpoint:
    s.it = tag_component_api.find_first(s.tag_mgr, TM_STATIC_HASH($$it))
  
  s.camera_tilt = 3.18f
  s.particle_entity = the_truth_assets_api.asset_object_from_path(s.tt, s.asset_root, "vfx/particles.entity")

  let camera = tag_component_api.find_first(s.tag_mgr, TM_STATIC_HASH("camera"))
  simulation_api.set_camera(s.simulation_ctx, camera)

  let root_entity = find_root_entity(s.entity_ctx, s.player)
  for i in 0..<8:
    let checkpoint_path = &"Checkpoints/checkpoint-{i+1}"
    let c: tm_entity_t = entity_api.resolve_path(s.entity_ctx, root_entity, checkpoint_path.cstring)
    TM_ASSERT(entity_api.is_alive(s.entity_ctx, c), error_api.def, &"Failed to find checkpoint {checkpoint_path}"):
      continue
    s.checkpoints_positions[i] = transform_component_api.get_position(s.trans_mgr, c)

  const singleton_name = "third_person_simulation_state"
  let gamestate = entity_api.gamestate(s.entity_ctx)
  let gs = tm_gamestate_singleton_t(
    name: singleton_name,
    size: sizeu32(simulate_persistent_state),
    serialize: serialize,
    deserialize: deserialize)
  
  gamestate_api.add_singleton(gamestate, gs, s)
  discard gamestate_api.deserialize_singleton(gamestate, singleton_name, s)
  s.rb = first_implementation(api_registry_api, tm_renderer_backend_i)

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

proc private_adjust_effect_start_color(s: ptr simulation_state_o, p: tm_entity_t, color: tm_vec3_t) =
  # Show how to poke at a custom shader variable ('start_color') exposed in a creation graph bound ot a specific draw call ('vfx')
  var rc = cast[ptr tm_render_component_public_t](entity_api.get_component(s.entity_ctx, p, s.render_component))
  let draw: ptr tm_creation_graph_draw_call_data_t = render_component_api.draw_call(rc, TM_STATIC_HASH("vfx"))

  if !draw.shader: return # this is buggy, draw is nil
  
  var res_buf: ptr tm_renderer_resource_command_buffer_o
  s.rb.create_resource_command_buffers(s.rb.inst, addr res_buf, 1)

  private_set_shader_constant(shader_api.shader_io(draw.shader), res_buf, addr draw.cbuffer, TM_STATIC_HASH("start_color"), addr color, sizeu32(color))

  gen submit, destroy:
    s.rb.`it resource_command_buffers`(s.rb.inst, addr res_buf, 1)

proc tick(state: ptr tm_simulation_state_o, args: ptr tm_simulation_frame_args_t) {.cdecl.} =
  let s = cast[ptr simulation_state_o](state)


  # Reset per-frame input
  gen x, y:
    s.input.mouse_delta.it = 0
  s.input.left_mouse_pressed = false

  # Read input
  var events: array[32, tm_input_event_t]
  while true:
    let n = input_api.events(s.processed_events, addr events[0], 32)
    for i in 0..<n:
      var e = events[i]
      if !e.source: continue
      if e.source.controller_type == TM_INPUT_CONTROLLER_TYPE_MOUSE:
        if e.item_id == TM_INPUT_MOUSE_ITEM_BUTTON_LEFT:
          let down = e.data.f.x > 0.5f
          s.input.left_mouse_pressed = down and not s.input.left_mouse_held
          s.input.left_mouse_held = down
        elif e.item_id == TM_INPUT_MOUSE_ITEM_MOVE:
          gen x, y:
            s.input.mouse_delta.it += e.data.f.it
      if e.source.controller_type == TM_INPUT_CONTROLLER_TYPE_KEYBOARD and e.type == TM_INPUT_EVENT_TYPE_DATA_CHANGE:
          s.input.held_keys[e.item_id] = e.data.f.x == 1.0f
    s.processed_events += n
    if n < 32:
      break
  # Capture mouse
  if ?args.ui:
    if not args.running_in_editor or (ui_api.is_hovering(args.ui, args.rect, 0) and s.input.left_mouse_pressed):
      s.mouse_captured = true
    
    if (args.running_in_editor and s.input.held_keys[TM_INPUT_KEYBOARD_ITEM_ESCAPE]) or not ui_api.window_has_focus(args.ui):
      s.mouse_captured = false
      application_api.set_cursor_hidden(application_api.application(), false)

    if s.mouse_captured:
      application_api.set_cursor_hidden(application_api.application(), true)

  let player_mover = cast[ptr tm_physx_mover_component_t](entity_api.get_component(s.entity_ctx, s.player, s.mover_component))

  TM_ASSERT(?player_mover, error_api.def, "Invalid player"): 
    return

  # For fudging jump timing
  if player_mover.is_standing:
    s.last_standing_time = args.time

  # Only allow input when mouse is captured
  if s.mouse_captured:
    # Exit on ESC
    if not args.running_in_editor and s.input.held_keys[TM_INPUT_KEYBOARD_ITEM_ESCAPE]:
      application_api.exit(application_api.application())
    
    # Camera pan control
    let
      mouse_sens = 0.03f * args.dt
      camera_pan_delta = -s.input.mouse_delta.x * mouse_sens
      pan_rot = tm_quaternion_from_rotation(vec3(y=1), camera_pan_delta)
    
    transform_component_api.set_rotation(s.trans_mgr, s.player, tm_quaternion_mul(pan_rot, transform_component_api.get_rotation(s.trans_mgr, s.player)))

    # Camera tilt control
    let camera_tilt_delta = -s.input.mouse_delta.y * mouse_sens
    s.camera_tilt = clamp(s.camera_tilt + camera_tilt_delta, 1.5f, 3.8f)
    let camera_pivot_rot =  tm_quaternion_from_euler(vec3(x = s.camera_tilt, z = -PI))
    transform_component_api.set_local_rotation(s.trans_mgr, s.camera_pivot, camera_pivot_rot)

    # Control animation state machine using input
    let
      smc = cast[ptr tm_animation_state_machine_component_t](entity_api.get_component(s.entity_ctx, s.player, s.anima_component))
      sm = smc.state_machine
    
    gen w, a, s, d, (run, leftshift):
      animation_state_machine_api.set_variable(sm, TM_STATIC_HASH($$it[0]), 
        if s.input.held_keys[`TM_INPUT_KEYBOARD_ITEM ^it[1]`]: 1f else: 0f)

    let can_jump = args.time < s.last_standing_time + 0.2f
    if can_jump and s.input.held_keys[TM_INPUT_KEYBOARD_ITEM_SPACE]:
      animation_state_machine_api.event(sm, TM_STATIC_HASH("jump"))
      player_mover.velocity.y += 6
      s.last_standing_time = 0

  # Check player against checkpoint
  gen (sphere, checkpoint), player:
    let `it[0] pos` = transform_component_api.get_position(s.trans_mgr, s.`it[1]`)
  
  if tm_vec3_length(tm_vec3_sub(sphere_pos, player_pos)) < 1.5f:
    inc s.current_checkpoint
    s.score += 1f

    if s.current_checkpoint == 8:
      s.current_checkpoint = 0
    
    if ?s.particle_entity.u64:
      # Spawn particle effect at position of next checkpoint.
      let p = entity_api.create_entity_from_asset(s.entity_ctx, s.particle_entity)
      # Set particle spawn location to next checkpoint.
      transform_component_api.set_position(s.trans_mgr, p, s.checkpoints_positions[s.current_checkpoint])

      # Make up an arbitrary color based on the direction the player entered the last checkpoint.
      var color = tm_vec3_normalize(tm_vec3_sub(sphere_pos, player_pos))
      color = vec3(abs(color.x), abs(color.y), abs(color.z))
      private_adjust_effect_start_color(s, p, color)

    transform_component_api.set_position(s.trans_mgr, s.checkpoint, s.checkpoints_positions[s.current_checkpoint])

  # UI
  if ?args.ui:
    var text = (&"You reached: {s.score} checkpoints").cstring
    var ui_label = tm_ui_label_t(rect: rect(5, 5, 20, 20), text: text)
    discard ui_api.label(args.ui, args.uistyle, addr ui_label)


var simulation_entry_i = tm_simulation_entry_i(
  id: TM_STATIC_HASH("tm_gameplay_sample_third_person_simulation_entry_i"),
  display_name: "Gameplay Sample Third Person",
  start: start,
  stop: stop,
  tick: tick
  )

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  api_registry_api = reg

  reg.get_api_for log, animation_state_machine_api, application_api, entity_api, error_api, input_api, 
    localizer_api, render_component_api, transform_component_api, shader_api, simulation_api, 
    ui_api, tag_component_api, the_truth_assets_api, gamestate_api

  reg.add_or_remove_impl load, simulation_entry_i
