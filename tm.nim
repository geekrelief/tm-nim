include tm_api_types
include tm_generated

template `$$`(s: untyped): untyped =
  cstring($s)

{.pragma: callback, exportc, cdecl, dynlib.}

proc NimMain() {.dynlib, exportc, cdecl, importc.}