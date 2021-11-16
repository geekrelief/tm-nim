## Tips for overriding
## ===================
##
## Invalid Pragma Error
## --------------------
## In tm_gen.nim, cImport uses `recurse = true`, this preprocesses the headers,
## flattening them, so we lose track of which file a type actually comes from.
## For example, if `a.h` includes `b.h` and `b.h` defines `foo`, when nimterop
## creates the header pragma `foo` will be marked as coming from `a.h` like `impaHdr`.
## If you override something, and reuse a nimterop custom `header` pragma you might
## run into an `invalid pragma` error.  Replace the nimterop custom `header` pragma,
## with a regular `header` pragma that points to the file where the definition exists.

## Opaque data: pointers ot `_o` or `_t` types.
## -----------
## TM uses a lot of opaque ptrs, typedefs with _o suffix, and other types that are passed
## between API calls. If all we have to do is pass these along between calls we can define
## an empty `object` for them.
## If TM, expects us to define our own data through an opaque type, we can override their
## definition using inheritance via `{.inheritable.}`.
## This makes it possible to create subtypes for opaque types to interop between Nim and C.

## Unions
## ------
## To handle unions, flatten the structure/union by bringing all the fields into the object.
## Rely on importc to handle all the fields correctly.

## TM_INHERITS
## -----------
## TM defines some `struct` types with another `struct` declaration at the top using TM_INHERITS.
## The workaround is to copy all fields from the referenced `struct` into the current `struct`.

cOverride:

  type
    tm_simulation_state_o* {.inheritable.} = object
    tm_component_manager_o* {.inheritable.} = object

    #>foundation/the_truth.h
    # check if we need to override for union if union is well behaved with importc
    tm_the_truth_property_definition_t* {.bycopy, union, header:tm_headers_dir & "foundation/the_truth.h", importc: "struct tm_the_truth_property_definition_t".} = object
      name*: cstring
      `type`*: tm_the_truth_property_type
      editor*: uint32
      #>union
      enum_editor*: tm_the_truth_editor_enum_t
      string_open_path_editor*: tm_the_truth_editor_string_open_path_t
      string_save_path_editor*: tm_the_truth_editor_string_save_path_t
      #<union
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

    #[
    #>foundation/input.h
    tm_input_data_t* {.bycopy, union, impinputHdr, importc: "struct tm_input_data_t".} = object
      f*: tm_vec4_t
      codepoint*: uint32
    #<foundation/input.h
    ]#

    #>foundation/buffer.h
    tm_streamable_buffers_i* {.bycopy, header: tm_headers_dir & "foundation/buffer.h", importc: "struct tm_streamable_buffers_i".} = object
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
    #tm_temp_allocator_api* {.bycopy, impcarrayHdr, importc: "struct tm_temp_allocator_api".} = object
    tm_temp_allocator_api* {.bycopy, header: tm_headers_dir & "foundation/api_types.h" ,importc: "struct tm_temp_allocator_api".} = object
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

    #>plugin/entity/entity.h
    tm_entity_system_i* {.bycopy, header:tm_headers_dir & "plugins/entity/entity.h", importc: "struct tm_entity_system_i".} = object
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

    tm_engine_i* {.header:tm_headers_dir & "plugins/entity/entity.h", importc: "struct tm_engine_i".} = object
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
    #<plugin/entity/entity.h

    #>plugin/renderer/resources.h

    tm_renderer_clear_value_t_color* {.union, bycopy, impresourcesHdr,
                                      importc: "struct color".} = object
      float32*: array[4, cfloat]
      int32*: array[4, int32]
      uint32*: array[4, uint32]
    #[
    tm_renderer_clear_value_t
    typedef union tm_renderer_clear_value_t
    {
        struct color
        {
            union
            {
                float float32[4];
                int32_t int32[4];
                uint32_t uint32[4];
            };
        } color;
        struct depth_stencil
        {
            float depth;
            uint32_t stencil;
        } depth_stencil;
    } tm_renderer_clear_value_t;
    ]#

    #[
    tm_renderer_clear_value_color* {.union, bycopy.} = object
      float32*: array[4, cfloat]
      int32*: array[4, int32]
      uint32*: array[4, uint32]

    tm_renderer_clear_value_depth_stencil* {.bycopy.} = object
      depth*: cfloat
      stencil*: uint32

    tm_renderer_clear_value_t* {.union, bycopy, impresourcesHdr, importc: "union tm_renderer_clear_value_t".} = object
      color*: tm_renderer_clear_value_color
      depth_stencil*: tm_renderer_clear_value_depth_stencil
      ]#
    #<plugin/renderer/resources.h

  #<type