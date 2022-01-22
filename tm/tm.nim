import std / [
  macros, 
  genasts, 
  strformat
  ]
import ptr_math, genit, nillean
export ptr_math, genit, nillean

# pragmas can't be exported from a module 
{.pragma: callback, exportc: "tm_load_plugin", cdecl, dynlib.}

proc NimMain*() {.dynlib, exportc, cdecl, importc.} # call NimMain on plugin load

# used to map nim procs to TM function typedef (which is not a function pointer!)
template tmType*(T: typedesc) {.pragma.}

template sizeu32*(x): untyped =
  sizeof(x).uint32

template sizeu64*(x): untyped =
  sizeof(x).uint64

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