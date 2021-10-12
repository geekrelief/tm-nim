include api_types
include tm_generated
include api_registry

template `$$`(s: untyped): untyped =
  cstring($s)

{.pragma: callback, exportc, cdecl, dynlib.}

proc NimMain() {.dynlib, exportc, cdecl, importc.}