template tm_get_api*(reg: ptr tm_api_registry_api, TYPE: untyped): untyped =
  cast[ptr `TYPE`](reg.get(astToStr(TYPE), `TYPE version`))

macro tm_get_api_for*(reg: ptr tm_api_registry_api, dests: varargs[typed]): untyped =
  doAssert(dests.len > 0, "Missing arguments")
  result = newNimNode(nnkStmtList)
  for dest in dests:
    let ptrType = getTypeInst(dest)
    let typeId = ptrType[0]
    let ast = genAst(reg, typeId, typeName = typeId.strVal, dest):
      dest = cast[ptr typeId](reg.get(typeName, `typeId version`))
    result.add ast

template tm_get_optional_api*(reg: ptr tm_api_registry_api, TYPE: untyped): untyped =
  cast[ptr `TYPE`](reg[].get_optional(astToStr(TYPE), `TYPE version`))

template tm_get_api_version*(reg: ptr tm_api_registry_api, TYPE: untyped, VERSION: tm_version_t): untyped =
  cast[ptr `TYPE`](reg[].get(astToStr(TYPE), VERSION))

template tm_set_or_remove_api*(reg: ptr tm_api_registry_api, load: bool, TYPE: untyped, impl: untyped): untyped = 
  if load:
    reg[].set(astToStr(TYPE), `TYPE version`, impl, sizeof(impl))
  else:
    reg[].remove(impl)

macro tm_add_or_remove_impl*(reg: ptr tm_api_registry_api, load: bool, impls: varargs[typed]): untyped =
  doAssert(impls.len > 0, "Missing impls")
  result = newNimNode(nnkStmtList)
  for impl in impls:
    let implType = getTypeImpl(impl)
    var (typeName, p) = case implType.kind
      of nnkObjectTy:
         (repr(getTypeInst(impl)), newCall(ident("unsafeAddr"), impl))
      of nnkProcTy:
        doAssert(impl.hasPragma("tm_type"), &"proc {impl.strVal} is missing \"tm_type\" pragma")
        var pragmaType = impl.getPragmaVal("tm_type")
        doAssert(pragmaType.symKind == nskType, &"proc {impl.strVal} \"tm_type\" argument must be a type")
        doAssert(pragmaType.getImpl[2].kind == nnkProcTy, &"{{.tm_type: {pragmaType.repr}.}} must be a proc type for: proc {impl.strVal}")
        (impl.getPragmaVal("tm_type").repr, impl)
      else:
        doAssert false, repr(impl) & " must be an object or proc type."
        ("", nil)

    var typeVersion = ident(typeName & "version")
    result.add genAst(reg, load, typeName, typeVersion, p) do:
      if load: 
        reg[].add_implementation(typeName, typeVersion, p)
      else:
        reg[].remove_implementation(typeName, typeVersion, p)