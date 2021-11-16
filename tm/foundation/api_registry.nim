template get_api*(reg: ptr tm_api_registry_api, t: typed{type}): untyped =
  cast[ptr `t`](reg.get(astToStr(t), `t version`))

macro get_api_for*(reg: ptr tmApiRegistryApi, dests: varargs[typed]{`var` & noalias}): untyped =
  assert(dests.len > 0, "Missing arguments")
  result = newNimNode(nnkStmtList)
  for dest in dests:
    let ptrType = getTypeInst(dest)
    assert(ptrType.kind == nnkPtrTy, &"{dest} must be a ptr to a TM type.")
    let t = ptrType[0]
    result.add genAst(reg, t, tname = t.strVal, dest) do:
      dest = cast[ptr t](reg.get(tname, `t version`))

#[
template tm_get_optional_api*(reg: ptr tm_api_registry_api, TYPE: untyped): untyped =
  cast[ptr `TYPE`](reg[].get_optional(astToStr(TYPE), `TYPE version`))

template tm_get_api_version*(reg: ptr tm_api_registry_api, TYPE: untyped, VERSION: tm_version_t): untyped =
  cast[ptr `TYPE`](reg[].get(astToStr(TYPE), VERSION))

template tm_set_or_remove_api*(reg: ptr tm_api_registry_api, load: bool, TYPE: untyped, impl: untyped): untyped = 
  if load:
    reg[].set(astToStr(TYPE), `TYPE version`, impl, sizeof(impl))
  else:
    reg[].remove(impl)
]#

macro add_or_remove_impl*(reg: ptr tmApiRegistryApi, load: bool, impls: varargs[typed]): untyped =
  assert(impls.len > 0, "Missing impls")
  result = newNimNode(nnkStmtList)
  for impl in impls:
    let t = getTypeImpl(impl)
    var (tname, p) = case t.kind:
      of nnkObjectTy:
         (repr(getTypeInst(impl)), newCall(ident("unsafeAddr"), impl))
      of nnkProcTy:
        assert(impl.hasCustomPragma(tmType), &"proc {impl.strVal} is missing \"tmType\" pragma")
        var pragmaType = impl.getCustomPragmaVal(tmType)
        assert(pragmaType.symKind == nskType, &"proc {impl.strVal} \"tmType\" argument must be a type")
        var tmType = pragmaType.getImpl()

        if tmType.kind == nnkTypeDef:
          while (tmType[2].kind == nnkSym):
            tmType = getImpl(tmType[2])

          assert(tmType.kind == nnkTypeDef and tmType[2].kind == nnkProcTy, &"{{.tmType: {pragmaType.repr}.}} must be a proc type for: proc {impl.strVal}")
        (pragmaType.repr, impl)
      else:
        raise newException(Defect, &"{impl.repr} must be an object or proc")

    result.add genAst(reg, load, tname, tversion = ident(tname & "version"), p) do:
      if load: 
        reg[].addImplementation(tname, tversion, p)
      else:
        reg[].removeImplementation(tname, tversion, p)