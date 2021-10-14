cOverride:
  # TM uses a lot of opaque ptrs, typedefs with _o suffix.
  # Overriding their definition using inheritance, object of RootObj,
  # makes it easy to create multiple definitions in Nim to interop with C.
  # So we can have one, tm_generated.nim, for multiple plugins.
  type
    tm_simulation_state_o* = object of RootObj
    tm_plugin_o* = object of RootObj
