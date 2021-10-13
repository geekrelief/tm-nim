template `$<`*(s: untyped): untyped =
  cstring($s)

{.pragma: callback, exportc, cdecl, dynlib.}

proc NimMain() {.dynlib, exportc, cdecl, importc.}

include foundation/api_types
include tm_generated
include foundation / api_registry
include foundation / allocator
