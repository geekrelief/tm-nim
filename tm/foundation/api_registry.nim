template tm_get_api*(reg: ptr tm_api_registry_api, TYPE: untyped): untyped =
  cast[ptr `TYPE`](reg.get(astToStr(TYPE), `TYPE version`))

macro tm_get_api_for*(reg: ptr tm_api_registry_api, dests: varargs[typed]): untyped =
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

template tm_set_or_remove_api(reg: ptr tm_api_registry_api, load: bool, TYPE: untyped, impl: untyped): untyped = 
  if load:
    reg[].set(astToStr(TYPE), `TYPE version`, impl, sizeof(impl))
  else:
    reg[].remove(impl)

macro tm_add_or_remove_impl(reg: ptr tm_api_registry_api, load: bool, impls: varargs[typed]): untyped =
  result = newNimNode(nnkStmtList)
  for impl in impls:
    #let impl = impls[0]
    let typeName = repr(getTypeInst(impl))
    let typeId = ident(typeName)
    let ast = genAst(reg, load, typeName, typeId, impl):
      let p = impl.unsafeAddr
      if load: 
        reg[].add_implementation(typeName, `typeId version`, p)
      else:
        reg[].remove_implementation(typeName, `typeId version`, p)
    result.add ast