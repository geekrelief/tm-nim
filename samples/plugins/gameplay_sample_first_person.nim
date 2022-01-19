include tm
import std / [random]


randomize()

const version = TM_VERSION(0, 1, 0)

var
  log: ptr tm_logger_api
  application_api: ptr tm_application_api
  draw2d_api: ptr tm_draw2d_api
  entity_api: ptr tm_entity_api
  error_api: ptr tm_error_api
  input_api: ptr tm_input_api
  #localizer_api: ptr tm_localizer_api
  physics_collision_api: ptr tm_physics_collision_api
  physx_scene_api: ptr tm_physx_scene_api
  #random_api: ptr tm_random_api
  simulation_api: ptr tm_simulation_api
  tag_component_api: ptr tm_tag_component_api
  temp_allocator_api: ptr tm_temp_allocator_api
  the_truth_assets_api: ptr tm_the_truth_assets_api
  tc_api: ptr tm_transform_component_api
  ui_api: ptr tm_ui_api
  gamestate_api: ptr tm_gamestate_api
  creation_graph_api: ptr tm_creation_graph_api

gen red, green, blue:
  const `it tag` = TM_STATIC_HASH("color_" & $$it)

type 
  input_state_t = object
    held_keys: array[TM_INPUT_KEYBOARD_ITEM_COUNT.uint64, bool]
    left_mouse_held: bool
    left_mouse_pressed: bool
    mouse_delta: tm_vec2_t

  Box_state = enum
    BOX_STATE_FREE
    BOX_STATE_CARRIED
    BOX_STATE_FLYING_UP
    BOX_STATE_FLYING_BACK
  
  Box_color = enum
    Red
    Green
    Blue

  simulation_state_o = object of tm_simulation_state_o
    allocator: ptr tm_allocator_i
    tt: ptr tm_the_truth_o # `tm_the_truth_api`
    entity_ctx: ptr tm_entity_context_o # `tm_entity_api`
    simulation_ctx: ptr tm_simulation_o # `tm_simulation_api`

    asset_root: tm_tt_id_t # interact with `tm_the_truth_assets_api`
    input: input_state_t

    # Entities
    player,
      player_camera,
      player_carry_anchor,
      box: tm_entity_t
    
    box_starting_point: tm_vec3_t
    box_starting_rot: tm_vec4_t

    box_state: Box_state

    box_color: Box_color

    box_fly_timer: float # Used to decide when to move box from BOX_STATE_FLYING_UP to BOX_STATE_FLYING_BACK

    # Current camera state
    look_yaw, look_pitch: float

    score: float

    # Misc
    processed_events: uint64
    player_collision_type: tm_tt_id_t
    box_collision_type: tm_tt_id_t

    # Component types
    mover_component,
      physics_shape_component,
      physics_joint_component,
      physx_rigid_body_component,
      physx_joint_component,
      tag_component,
      transform_component: tm_component_type_t

    # Component managers
    trans_man: ptr tm_transform_component_manager_o
    tag_man: ptr tm_tag_component_manager_o

    mouse_captured: bool
    box_interactable: bool
  #> simulation_state_o

  simulate_persistent_state = object
    player,
      player_camera,
      player_carry_anchor,
      box: tm_gamestate_object_id_t
    
    box_starting_point: tm_vec3_t
    box_starting_rot: tm_vec4_t

    box_state: Box_state
    box_color: Box_color

    box_fly_timer: float

    look_yaw, look_pitch: float

    score: float
  #>simulate_persistent_state

proc serialize(s, d: pointer) {.cdecl.} =
  var 
    src = cast[ptr simulation_state_o](s)
    dst = cast[ptr simulate_persistent_state](d)
  
  gen player, player_camera, player_carry_anchor, box:
    discard entity_api.get_entity_persistent_id(src.entity_ctx, src.it, addr dst.it)
  
  gen box_starting_point, box_starting_rot, box_state, box_color, box_fly_timer, look_yaw, look_pitch, score:
    dst.it = src.it

proc deserialize(d, s: pointer) {.cdecl.} =
  var
    dst = cast[ptr simulation_state_o](d)
    src = cast[ptr simulate_persistent_state](s)
  
  gen player, player_camera, player_carry_anchor, box:
    dst.it = entity_api.lookup_entity_from_gamestate_id(dst.entity_ctx, addr src.it)
  
  gen box_starting_point, box_starting_rot, box_state, box_color, box_fly_timer, look_yaw, look_pitch, score:
    dst.it = src.it

  simulation_api.set_camera(dst.simulation_ctx, dst.player_camera)

proc update_box_material(state: ptr simulation_state_o) =
  let box: tm_entity_t = state.box

  if box.u64 != 0:
    var material: tm_tt_id_t = gen(red, green, blue):
      case state.box_color:
        of `^it`: the_truth_assets_api.asset_object_from_path(state.tt, state.asset_root, "materials/box-" & $$it & "-mat.creation")
    
    let cube = box
    if cube.u64 == 0: return

    var ta = temp_allocator_api.init()
    var instances = creation_graph_api.get_instances_from_component(state.tt, state.entity_ctx, cube, TM_TT_TYPE_HASH_RENDER_COMPONENT, ta)

    var cg_ctx = tm_creation_graph_context_t(
      tt: state.tt,
      entity_ctx: state.entity_ctx,
      ta: ta,
      entity_id: box.u64,
      device_affinity_mask: TM_RENDERER_DEVICE_AFFINITY_MASK_ALL)

    var mat = tm_resource_reference_t(
      creation_graph: material,
      node_type_hash: TM_CREATION_GRAPH_SHADER_INSTANCE_OUTPUT_HASH
    )

    for instance in instances.items(tm_carray_size(instances)):
      creation_graph_api.set_input_value(instance, addr cg_ctx, TM_STATIC_HASH("material"), addr mat, sizeof(mat).uint32)

proc change_box_to_random_color(state: ptr simulation_state_o) =
  var box = state.box

  var tags = [(redTag, Red), (greenTag, Green), (blueTag, Blue)]
  var newTag = tags.sample
  while (tag_component_api.has_tag(state.tag_man, box, newTag[0])):
    newTag = tags.sample
  
  state.box_color = newTag[1]

  gen red, green, blue:
    tag_component_api.remove_tag(state.tag_man, box, `it tag`)
  
  tag_component_api.add_tag(state.tag_man, box, newTag[0])

  update_box_material(state)

proc start(args: ptr tm_simulation_start_args_t): ptr tm_simulation_state_o {.cdecl.} =
  result = args.allocator.alloc(simulation_state_o(
    allocator: args.allocator,
    tt: args.tt,
    entity_ctx: args.entity_ctx,
    simulation_ctx: args.simulation_ctx,
    asset_root: args.asset_root))
  var s = cast[ptr simulation_state_o](result)

  gen(c = component):
    s.`mover c` = entity_api.`lookup c type`(s.entity_ctx, `TM_TT_TYPE_HASH_PHYSX_MOVER c`)
    gen physics_joint, physics_shape, physx_joint, physx_rigid_body, tag, transform:
      s.`it c` = entity_api.`lookup c type`(s.entity_ctx, `TM_TT_TYPE_HASH it c`)

    gen (trans, transform), (tag, tag):
      s.`it[0] man` = cast[ptr `tm it[1] c manager_o`](entity_api.`c manager`(s.entity_ctx, s.`it[1] c`))

    gen player, player_camera, player_carry_anchor:
      s.it = `tag c api`.find_first(s.tag_man, TM_STATIC_HASH($$it))
  
  simulation_api.set_camera(s.simulation_ctx, s.player_camera)

  s.box = tag_component_api.find_first(s.tag_man, TM_STATIC_HASH("box"))
  change_box_to_random_color(s)
  let box_trans = cast[ptr tm_transform_component_t](entity_api.get_component(s.entity_ctx, s.box, s.transform_component))
  s.box_starting_point = box_trans.world.pos
  s.box_starting_rot = box_trans.world.rot

  var ta = temp_allocator_api.init()
  let collision_types: ptr tm_physics_collision_t = physics_collision_api.find_all(s.tt, ta)
  for c in carray_items(collision_types):
    if c.name == TM_STATIC_HASH("player"):
      s.player_collision_type = c.collision

    if c.name == TM_STATIC_HASH("box"):
      s.box_collision_type = c.collision

  const singleton_name = "first_person_simulation_state"
  var gamestate = entity_api.gamestate(s.entity_ctx)
  var gs = tm_gamestate_singleton_t(
    name: singleton_name,
    size: sizeof(simulate_persistent_state).uint32,
    serialize: serialize,
    deserialize: deserialize)
  
  gamestate_api.add_singleton(gamestate, gs, s)
  discard gamestate_api.deserialize_singleton(gamestate, singleton_name, s)

proc stop(state: ptr tm_simulation_state_o, commands: ptr tm_entity_commands_o) {.cdecl.} =
  var s = cast[ptr simulation_state_o](state)
  s.allocator.free(s)

proc tick(state: ptr tm_simulation_state_o, args: ptr tm_simulation_frame_args_t) {.cdecl.} =

  var s = cast[ptr simulation_state_o](state)
  # Reset per-frame-input
  gen x, y:
    s.input.mouse_delta.it = 0
  s.input.left_mouse_pressed = false

  # Read input
  var 
    events: array[32, tm_input_event_t]
    mouse_captured_this_frame = s.mouse_captured
  while (true):
    let n = input_api.events(s.processed_events, addr events[0], 32)

    for e in events:
      if e.source.isNil: continue
      if mouse_captured_this_frame:
        if e.source.controller_type == TM_INPUT_CONTROLLER_TYPE_MOUSE:
          if e.item_id == TM_INPUT_MOUSE_ITEM_BUTTON_LEFT:
            let down = e.data.f.x > 0.5f
            s.input.left_mouse_pressed = down and not s.input.left_mouse_held
            s.input.left_mouse_held = down
          elif e.item_id == TM_INPUT_MOUSE_ITEM_MOVE:
            gen x, y:
              s.input.mouse_delta.it += e.data.f.it

        if e.source.controller_type == TM_INPUT_CONTROLLER_TYPE_KEYBOARD and e.`type` == TM_INPUT_EVENT_TYPE_DATA_CHANGE:
          s.input.held_keys[e.item_id] = e.data.f.x == 1.0f
      else:
        if e.source.controller_type == TM_INPUT_CONTROLLER_TYPE_MOUSE and e.item_id == TM_INPUT_MOUSE_ITEM_BUTTON_LEFT:
          let down = e.data.f.x > 0.5f
          s.input.left_mouse_pressed = down and not s.input.left_mouse_held
          s.input.left_mouse_held = down

        if e.source.controller_type == TM_INPUT_CONTROLLER_TYPE_KEYBOARD and
          e.item_id == TM_INPUT_KEYBOARD_ITEM_ESCAPE and e.`type` == TM_INPUT_EVENT_TYPE_DATA_CHANGE:
          s.input.held_keys[e.item_id] = e.data.f.x == 1.0f

    s.processed_events += n
    if n < 32:
      break

  # Capture mouse
  if args.ui != nil:
    if not args.running_in_editor or ui_api.is_hovering(args.ui, args.rect, 0) and s.input.left_mouse_pressed:
      s.mouse_captured = true
    if args.running_in_editor and s.input.held_keys[TM_INPUT_KEYBOARD_ITEM_ESCAPE] or not ui_api.window_has_focus(args.ui):
      s.mouse_captured = false
      application_api.set_cursor_hidden(application_api.application(), false)
    if s.mouse_captured:
      application_api.set_cursor_hidden(application_api.application(), true)

  gen (pos, position), (rot, rotation):
    let `camera it[0]` = tc_api.`get it[1]`(s.trans_man, s.player_camera)
  let player_mover = cast[ptr tm_physx_mover_component_t](entity_api.get_component(s.entity_ctx, s.player, s.mover_component))

  TM_ASSERT(player_mover != nil, error_api.def, "Invalid player"):
    return

  # Process input if mouse is captured
  if s.mouse_captured:
    # Exit on ESC
    if not args.running_in_editor and s.input.held_keys[TM_INPUT_KEYBOARD_ITEM_ESCAPE]:
      application_api.exit(application_api.application())
    
    var local_movement: tm_vec3_t
    gen (A, `-=`, x), (D, `+=`, x), (W, `-=`, z), (S, `+=`, z):
      if s.input.held_keys[`TM_INPUT_KEYBOARD_ITEM it[0]`]:
        it[1](local_movement.`it[2]`, 1.0f)
    
    # Move
    if tm_vec3_length(local_movement) != 0f:
      var rotated_movement = tm_quaternion_rotate_vec3(camera_rot, local_movement)
      rotated_movement.y = 0
      let 
        normalized_rotated_movement = tm_vec3_normalize(rotated_movement)
        final_movement = tm_vec3_mul(normalized_rotated_movement, 5)
      gen x, z:
        player_mover.velocity.it = final_movement.it
    else:
      gen x, z:
        player_mover.velocity.it = 0
    
    # Look
    let mouse_sens = 0.1f * args.dt
    s.look_yaw -= s.input.mouse_delta.x * mouse_sens
    s.look_pitch -= s.input.mouse_delta.y * mouse_sens
    s.look_pitch = clamp(s.look_pitch, -TM_PI / 3f, TM_PI / 3f)
    let
      yawq = tm_quaternion_from_rotation(vec3(y = 1f), s.look_yaw)
      local_sideways = tm_quaternion_rotate_vec3(yawq, vec3(x = 1f))
      pitchq = tm_quaternion_from_rotation(local_sideways, s.look_pitch)
    tc_api.set_local_rotation(s.trans_man, s.player_camera, tm_quaternion_mul(pitchq, yawq))

    # Jump
    if s.input.held_keys[TM_INPUT_KEYBOARD_ITEM_SPACE] and player_mover.is_standing:
      player_mover.velocity.y = 15

  # Box carry anchor is kinematic physics body (so we can put joints on it), move it manually
  let 
    physx_scene = args.physx_scene
    camera_forward = tm_quaternion_rotate_vec3(camera_rot, vec3(z = -1f))
    anchor_pos = tm_vec3_add(tm_vec3_add(camera_pos, tm_vec3_mul(camera_forward, 1.5f)), tm_vec3_mul(player_mover.velocity, args.dt))

  tc_api.set_position(s.trans_man, s.player_carry_anchor, anchor_pos)
  tc_api.set_rotation(s.trans_man, s.player_carry_anchor, camera_rot)

  # Update box color if necessary
  update_box_material(s)
  s.box_interactable = false

  # Box state machine
  case s.box_state:
  of BOX_STATE_FREE:
    # Check if box is in a drop zone that has the same color as itself
    var 
      touching_correct_drop_zone = false
    for t in carray_mitems(physx_scene_api.on_contact(physx_scene)):
      let 
        e0 = t.actor_0
        e1 = t.actor_1
      
      if e0.u64 != s.box.u64 and e1.u64 != s.box.u64: continue

      gen(t = tag_component_api, red, green, blue):
        let `on it` = t.has_tag(s.tag_man, e0, `it tag`) and t.has_tag(s.tag_man, e1, `it tag`)
      if not (onRed or onGreen or onBlue): continue

      touching_correct_drop_zone = true

    let box_pos = tc_api.get_position(s.trans_man, s.box)
    if touching_correct_drop_zone:
      # If box is in correct drop zone and has low velocity, send it flying upwards.
      let box_velocity = physx_scene_api.velocity(physx_scene, s.box)
      if tm_vec3_length(box_velocity) < 0.01f:
        physx_scene_api.add_force(physx_scene, s.box, vec3(y = 10f), TM_PHYSX_FORCE_FLAGS_VELOCITY_CHANGE)
        s.box_fly_timer = 0.7f
        s.box_state = BOX_STATE_FLYING_UP
        s.score += 1f
    elif box_pos.y < -10.0f:
      physx_scene_api.set_velocity(physx_scene, s.box, vec3(y = 20f))
      s.box_fly_timer = 1f
      s.box_state = BOX_STATE_FLYING_UP
    else:
      # If box is not in correct drop zone and player clicks left mouse button, try picking it up using raycast.
      let r = physx_scene_api.raycast(physx_scene, camera_pos, camera_forward, 2.5f, s.player_collision_type, tm_physx_raycast_flags_t(), nil, 0)
      if r.has_block:
        let hit = r.block.body
        if s.box.u64 == hit.u64:
          s.box_interactable = true
          if s.input.left_mouse_pressed:
            var shape = cast[ptr tm_physics_shape_component_t](entity_api.get_component(s.entity_ctx, s.box, s.physics_shape_component))
            shape.collision_id = s.player_collision_type
            # Forces re-mirroring of physx rigid body, so physx shape gets correct collision type.
            entity_api.remove_component(s.entity_ctx, s.box, s.physx_rigid_body_component)

            tc_api.set_position(s.trans_man, s.box, anchor_pos)
            var j = cast[ptr tm_physics_joint_component_t](entity_api.add_component(s.entity_ctx, s.box, s.physics_joint_component))
            j.joint_type = TM_PHYSICS_JOINT_FIXED
            j.body_0 = s.box
            j.body_1 = s.player_carry_anchor
            s.box_state = BOX_STATE_CARRIED
  of BOX_STATE_CARRIED:
    if s.input.left_mouse_pressed:
      # Drop box
      gen physics, physx:
        entity_api.remove_component(s.entity_ctx, s.box, s.`it joint_component`)
      var shape = cast[ptr tm_physics_shape_component_t](entity_api.get_component(s.entity_ctx, s.box, s.physics_shape_component))
      shape.collision_id = s.box_collision_type

      # Forces re-mirroring of physx rigid body, so the physx shape gets correct collision type.
      entity_api.remove_component(s.entity_ctx, s.box, s.physx_rigid_body_component)

      physx_scene_api.set_kinematic(physx_scene, s.box, false)
      physx_scene_api.add_force(physx_scene, s.box, tm_vec3_mul(camera_forward, 1500 * args.dt), TM_PHYSX_FORCE_FLAGS_IMPULSE)
      s.box_state = BOX_STATE_FREE

  of BOX_STATE_FLYING_UP:
    s.box_fly_timer -= args.dt
    if s.box_fly_timer <= 0.0001f:
      physx_scene_api.set_kinematic(physx_scene, s.box, true)
      change_box_to_random_color(s)
      s.box_state = BOX_STATE_FLYING_BACK

  of BOX_STATE_FLYING_BACK:
    # This state interpolates the box back to its initial position and changes the
    # color once it reaches it.
    let
      box_pos = tc_api.get_position(s.trans_man, s.box)
      box_to_spawn = tm_vec3_sub(s.box_starting_point, box_pos)
      spawn_point_dir = tm_vec3_normalize(box_to_spawn)

    change_box_to_random_color(s)
      
    if tm_vec3_length(box_to_spawn) < 0.1f:
      tc_api.set_position(s.trans_man, s.box, s.box_starting_point)
      physx_scene_api.set_kinematic(physx_scene, s.box, false)
      physx_scene_api.set_velocity(physx_scene, s.box, vec3())
      s.box_state = BOX_STATE_FREE
    else:
      let interoplate_to_start_pos = tm_vec3_add(box_pos, tm_vec3_mul(spawn_point_dir, args.dt * 10))
      tc_api.set_position(s.trans_man, s.box, interoplate_to_start_pos)

  if args.ui != nil:
    discard
    # UI: Score
    var ui_label = tm_ui_label_t(
      rect: rect(5f, 5f, 20f, 20f), 
      text: cstring(&"This box has been correctly placed {s.score:1} times")
      )
    discard ui_api.label(args.ui, args.uistyle, addr ui_label)

    # UI: Crosshair
    var
      uib = ui_api.buffers(args.ui)
      crosshair_pos = vec2(args.rect.w / 2f, args.rect.h / 2f)
      style:tm_draw2d_style_t
    discard ui_api.to_draw_style(args.ui, addr style, args.uistyle)
    style.color = if s.box_interactable or s.box_state == BOX_STATE_CARRIED: 
        color(255, 255, 255, 255) 
      else: 
        color(120, 120, 120, 255)
    draw2d_api.fill_circle(uib.vbuffer, uib.ibuffers[TM_UI_BUFFER_MAIN], addr style, crosshair_pos, 3)

var simulation_entry_i = tm_simulation_entry_i(
  id: TM_STATIC_HASH("tm_gameplay_sample_first_person_simulation_entry_i"),
  display_name: "Gameplay Sample First Person",
  start: start,
  stop: stop,
  tick: tick)

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  reg.get_api_for log, application_api, entity_api, physics_collision_api, simulation_api, 
    tag_component_api, temp_allocator_api, gamestate_api, the_truth_assets_api, creation_graph_api,
    input_api, ui_api, tc_api, physx_scene_api, draw2d_api

  reg.add_or_remove_impl load, simulation_entry_i