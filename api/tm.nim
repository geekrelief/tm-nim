doAssert defined(windows), "Only tested on Windows"

include tm_api_types
include tm_generated
include tm_api_registry

template `$$`(s: untyped): untyped =
  cstring($s)

{.pragma: callback, exportc, cdecl, dynlib.}

proc NimMain() {.dynlib, exportc, cdecl, importc.}