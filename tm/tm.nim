import std / [
  macros, 
  genasts, 
  strformat
  ]
import ptr_math, genit
export ptr_math, genit

macro callback*(def: untyped): untyped =
  # adds {.exportc:"tm_load_plugin", cdecl, dynlib.}
  # pragmas can't be exported from a module 
  def.pragma = nnkPragma.newTree( nnkExprColonExpr.newTree(ident("exportc"), newLit("tm_load_plugin")), ident("cdecl"), ident("dynlib"))
  def

proc NimMain*() {.dynlib, exportc, cdecl, importc.} # call NimMain on plugin load

# used to map nim procs to TM function typedef (which is not a function pointer!)
template tmType*(T: typedesc) {.pragma.}

func toArray*[T](n: static uint, a: varargs[T]): array[n, T] =
  for i, x in a:
    result[i] = x

include foundation / api_types
include tm_generated
include foundation / [
  log,
  error,
  api_registry, 
  allocator, 
  temp_allocator,
  localizer,
  the_truth,
  carray
  ]

include plugin / [ entity ]