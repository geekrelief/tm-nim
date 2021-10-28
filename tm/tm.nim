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

func toArray*[T](n: static int, a: varargs[T]): array[n, T] =
  for i, x in a:
    result[i] = x

iterator items*[T](starting: ptr T, ending: ptr T): lent T =
  var p = starting
  while p != ending:
    yield p[] 
    p += 1

iterator mitems*[T](starting: ptr T, ending: ptr T): var T =
  var p = starting
  while p != ending:
    yield p[] 
    p += 1

iterator items*[T](uarray: UncheckedArray[T] | ptr T, len: SomeInteger): lent T =
  for i in 0..<len:
    yield uarray[i]

iterator mitems*[T](uarray: UncheckedArray[T] | ptr T, len: SomeInteger): var T =
  for i in 0..<len:
    yield uarray[i]

iterator pairs*[T](uarray: UncheckedArray[T] | ptr T, len: SomeInteger): (SomeInteger, lent T) =
  for i in 0..<len:
    yield (i, uarray[i])

iterator mpairs*[T](uarray: UncheckedArray[T] | ptr T, len: SomeInteger): (SomeInteger, var T) =
  for i in 0..<len:
    yield (i, uarray[i])


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