cOverride:
  # TM uses a lot of opaque ptrs, typedefs with _o suffix.
  # Overriding their definition using inheritance, object of RootObj,
  # makes it easy to create multiple definitions in Nim to interop with C.
  # So we can have one, tm_generated.nim, for multiple plugins.
  type
    tm_simulation_state_o* {.inheritable.} = object
    tm_plugin_o* = object
    tm_temp_allocator_o* = object

    tm_the_truth_property_definition_t* {.bycopy, union, impthe_truthHdr, importc: "struct tm_the_truth_property_definition_t".} = object
      name*: cstring
      `type`*: tm_the_truth_property_type
      editor*: uint32
      enum_editor*: tm_the_truth_editor_enum_t
      string_open_path_editor*: tm_the_truth_editor_string_open_path_t
      string_save_path_editor*: tm_the_truth_editor_string_save_path_t
      type_hash*: tm_strhash_t
      allow_other_types*: bool
      padding_451*: array[7, cchar]
      buffer_extension*: cstring
      buffer_extension_f*: proc (tt: ptr tm_the_truth_o; `object`: tm_tt_id_t;
                                property: uint32): cstring {.cdecl.}
      tooltip*: cstring
      not_serialized*: bool
      padding_469*: array[7, cchar]
      ui_name*: cstring

    #tm_the_truth_create_types_i* = proc (tt: ptr tm_the_truth_o) {.cdecl.}