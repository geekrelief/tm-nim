cOverride:

  ## Tips for overriding

  ## Opaque ptr (_o): TM uses a lot of opaque ptrs, typedefs with _o suffix.
  ## Overriding their definition using inheritance via {.inheritable.}
  ## makes it easy to create multiple definitions in Nim to interop with C.
  ## So we can have one, tm_generated.nim, for multiple plugins.

  ## Unions: To handle unions, flatten the structure/union by bringing all
  ## the fields into the object.
  ## Rely on importc to handle all the fields correctly.

  ## TM_INHERITS: Copy all fields from super struct into sub struct.

  type
    tm_simulation_state_o* {.inheritable.} = object
    tm_component_manager_o* {.inheritable.} = object # entity.h
    tm_plugin_o* = object
    tm_temp_allocator_o* = object

    #>foundation/the_truth.h
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
      buffer_extension_f*: proc (tt: ptr tm_the_truth_o; `object`: tm_tt_id_t; property: uint32): cstring {.cdecl.}
      tooltip*: cstring
      not_serialized*: bool
      padding_469*: array[7, cchar]
      ui_name*: cstring
    #<foundation/the_truth.h

    tm_streamable_buffers_i* {.bycopy, impbufferHdr, importc: "struct tm_streamable_buffers_i".} = object
      #>inherit tm_buffers_i
      inst*: ptr tm_buffers_o
      allocate*: proc (inst: ptr tm_buffers_o; size: uint64; initialize: pointer): pointer {.cdecl.}
      add*: proc (inst: ptr tm_buffers_o; data: pointer; size: uint64; hash: uint64): uint32 {.cdecl.}
      retain*: proc (inst: ptr tm_buffers_o; id: uint32) {.cdecl.}
      release*: proc (inst: ptr tm_buffers_o; id: uint32) {.cdecl.}
      get*: proc (inst: ptr tm_buffers_o; id: uint32; size: ptr uint64): pointer {.cdecl.}
      size*: proc (inst: ptr tm_buffers_o; id: uint32): uint64 {.cdecl.}
      hash*: proc (inst: ptr tm_buffers_o; id: uint32): uint64 {.cdecl.}
      lookup*: proc (inst: ptr tm_buffers_o; hash: uint64): uint32 {.cdecl.}
      debug_refcount*: proc (inst: ptr tm_buffers_o; id: uint32): uint32 {.cdecl.}
      #<inherit tm_buffers_i

      map*: proc (inst: ptr tm_buffers_o; path: cstring; offset: uint64; size: uint64; hash: uint64): uint32 {.cdecl.}
      map_database*: proc (inst: ptr tm_buffers_o; hash: uint64; size: uint64; file: ptr tm_file_o; page_size: uint32; page_header_size: uint32; first_page: uint32): uint32 {.cdecl.}
      is_mapped*: proc (inst: ptr tm_buffers_o; id: uint32): bool {.cdecl.}
      is_loaded*: proc (inst: ptr tm_buffers_o; id: uint32): bool {.cdecl.}
      unload*: proc (inst: ptr tm_buffers_o; id: uint32) {.cdecl.}
      save*: proc (inst: ptr tm_buffers_o; id: uint32; path: cstring; offset: uint64) {.cdecl.}
      background_load_all*: proc (inst: ptr tm_buffers_o; percentage: ptr cfloat) {.cdecl.}
      ensure_all_loaded*: proc (inst: ptr tm_buffers_o) {.cdecl.}
      set_io*: proc (inst: ptr tm_buffers_o; io: ptr tm_os_file_io_api) {.cdecl.}
    #<foundation/buffer.h
    
    #>foundation/temp_allocator.h
    tm_temp_allocator_api* {.bycopy, impcarrayHdr, importc: "struct tm_temp_allocator_api".} = object
      create*: proc (backing: ptr tm_allocator_i): ptr tm_temp_allocator_i {.cdecl.}
      # create_in_buffer (buffer: cstring)
      create_in_buffer*: proc (buffer: ptr cchar; size: uint64; backing: ptr tm_allocator_i): ptr tm_temp_allocator_i {.cdecl.}
      destroy*: proc (ta: ptr tm_temp_allocator_i) {.cdecl.}
      allocator*: proc (a: ptr tm_allocator_i; ta: ptr tm_temp_allocator_i) {.cdecl.}
      frame_alloc*: proc (size: uint64): pointer {.cdecl.}
      frame_allocator*: proc (): ptr tm_allocator_i {.cdecl.}
      tick_frame*: proc () {.cdecl.}
      printf*: proc (ta: ptr tm_temp_allocator_i; format: cstring): cstring {.cdecl, varargs.}
      vprintf*: proc (ta: ptr tm_temp_allocator_i; format: cstring; args: va_list): cstring {.cdecl.}
      frame_printf*: proc (format: cstring): cstring {.cdecl, varargs.}
      frame_vprintf*: proc (format: cstring; args: va_list): cstring {.cdecl.}
      statistics*: ptr tm_temp_allocator_statistics_t
    #<foundation/temp_allocator.h

    #>plugin/entity.h

    tm_entity_system_i* {.bycopy, impentityHdr, importc: "struct tm_entity_system_i".} = object
      #>inherit tm_engine_system_common_i
      ui_name*: cstring
      hash*: tm_strhash_t
      disabled*: bool
      exclusive*: bool
      padding_448*: array[2, cchar]
      num_components*: uint32
      components*: array[32, tm_component_type_t]
      writes*: array[32, bool]
      before_me*: array[TM_MAX_DEPENDENCIES_FOR_ENGINE, tm_strhash_t]
      after_me*: array[TM_MAX_DEPENDENCIES_FOR_ENGINE, tm_strhash_t]
      phase*: tm_strhash_t
      #<inherit tm_engine_system_common_i
      
      inst*: ptr tm_entity_system_o
      inited*: bool
      padding_553*: array[7, cchar]
      init*: proc (ctx: ptr tm_entity_context_o; inst: ptr tm_entity_system_o; commands: ptr tm_entity_commands_o) {.cdecl.}
      update*: proc (ctx: ptr tm_entity_context_o; inst: ptr tm_entity_system_o; commands: ptr tm_entity_commands_o) {.cdecl.}
      shutdown*: proc (ctx: ptr tm_entity_context_o; inst: ptr tm_entity_system_o; commands: ptr tm_entity_commands_o) {.cdecl.}
      hot_reload*: proc (ctx: ptr tm_entity_context_o; inst: ptr tm_entity_system_o; commands: ptr tm_entity_commands_o) {.cdecl.}

    tm_engine_i* {.impentityHdr, importc: "struct tm_engine_i".} = object
      #>inherit tm_engine_system_common_i
      ui_name*: cstring
      hash*: tm_strhash_t
      disabled*: bool
      exclusive*: bool
      padding_448*: array[2, cchar]
      num_components*: uint32
      components*: array[32, tm_component_type_t]
      writes*: array[32, bool]
      before_me*: array[TM_MAX_DEPENDENCIES_FOR_ENGINE, tm_strhash_t]
      after_me*: array[TM_MAX_DEPENDENCIES_FOR_ENGINE, tm_strhash_t]
      phase*: tm_strhash_t
      #<inherit tm_engine_system_common_i

      inst*: ptr tm_engine_o
      update*: proc (inst: ptr tm_engine_o; data: ptr tm_engine_update_set_t; commands: ptr tm_entity_commands_o) {.cdecl.}
      num_excluded*: uint32
      padding_504*: array[4, cchar]
      excluded*: array[32, tm_component_type_t]
      filter*: proc (inst: ptr tm_engine_o; components: ptr tm_component_type_t; num_components: uint32; mask: ptr tm_component_mask_t): bool {.cdecl.}

    #<plugin/entity.h
  #<type