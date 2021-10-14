{.pragma: callback, exportc, cdecl, dynlib.}

proc NimMain() {.dynlib, exportc, cdecl, importc.}

import std / genasts

include foundation/api_types
include tm_generated
include foundation / api_registry
include foundation / allocator
include foundation / log