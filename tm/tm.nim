import std / [
  macros{.all.}, 
  genasts, 
  strformat
  ]
import ptr_math
export ptr_math

macro callback*(def: untyped): untyped =
  # adds {.exportc:"tm_load_plugin", cdecl, dynlib.}
  # pragmas can't be exported from a module 
  def.pragma = nnkPragma.newTree( nnkExprColonExpr.newTree(ident("exportc"), newLit("tm_load_plugin")), ident("cdecl"), ident("dynlib"))
  def

proc NimMain*() {.dynlib, exportc, cdecl, importc.} # call NimMain on plugin load

#used in macros
proc hasPragma*(n: NimNode, cpName: string): bool =
  let pragmaNode = customPragmaNode(n)
  for p in pragmaNode:
    if (p.kind == nnkSym and p.strVal.eqIdent(cpName)) or
        (p.kind in nnkPragmaCallKinds and p.len > 0 and p[0].kind == nnkSym and p[0].strVal.eqIdent(cpName)):
      return true
  return false

#used in macros
proc getPragmaVal*(n: NimNode, cpName: string):NimNode =
  result = nil
  let pragmaNode = customPragmaNode(n)
  for p in pragmaNode:
    if p.kind in nnkPragmaCallKinds and p.len > 0 and p[0].kind == nnkSym and p[0].strVal.eqIdent(cpName):
      if p.len == 2 or (p.len == 3 and p[1].kind == nnkSym and p[1].symKind == nskType):
        result = p[1]
      else:
        let def = p[0].getImpl[3]
        result = newTree(nnkPar)
        for i in 1 ..< def.len:
          let key = def[i][0]
          let val = p[i]
          result.add newTree(nnkExprColonExpr, key, val)
      break
  if result.kind == nnkEmpty:
    error(n.repr & " doesn't have a pragma named " & cpName) # returning an empty node results in most cases in a cryptic error,

# used to map nim procs to TM function typedef (which is not a function pointer!)
template tmType*(T: typedesc) {.pragma.}

func toArray*[T](n: static uint, a: varargs[T]): array[n, T] =
  for i, x in a:
    result[i] = x

include foundation / api_types
include tm_generated
include foundation / [
  log,
  api_registry, 
  allocator, 
  temp_allocator,
  localizer,
  the_truth,
  carray
  ]

include plugin / [ entity ]