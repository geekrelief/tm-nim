template tm_get_api*(reg: ptr tm_api_registry_api, TYPE: untyped): untyped =
  cast[ptr `TYPE`](reg.get(astToStr(TYPE), `TYPE version`))

macro tm_get_api_for*(reg: ptr tm_api_registry_api, ptrDest: typed): untyped =
  let ptrType = getTypeInst(ptrDest)
  let typeId = ptrType[0]
  result = genAst(reg, typeId, typeName = typeId.strVal, ptrDest):
    ptrDest = cast[ptr typeId](reg.get(typeName, `typeId version`))

template tm_get_optional_api*(reg: ptr tm_api_registry_api, TYPE: untyped): untyped =
  cast[ptr `TYPE`](reg[].get_optional(astToStr(TYPE), `TYPE version`))

template tm_get_api_version*(reg: ptr tm_api_registry_api, TYPE: untyped, VERSION: tm_version_t): untyped =
  cast[ptr `TYPE`](reg[].get(astToStr(TYPE), VERSION))

template tm_set_or_remove_api(reg: ptr tm_api_registry_api, load: bool, TYPE: untyped, impl: untyped): untyped = 
  if load:
    reg[].set(astToStr(TYPE), `TYPE version`, impl, sizeof(impl))
  else:
    reg[].remove(impl)

macro tm_add_or_remove_impl(reg: ptr tm_api_registry_api, load: bool, impl: typed): untyped =
  let typeName = repr(getTypeInst(impl))
  let typeId = ident(typeName)
  result = genAst(reg, load, typeName, typeId, impl):
    let p = impl.addr
    if load: 
      reg[].add_implementation(typeName, `typeId version`, p)
    else:
      reg[].remove_implementation(typeName, `typeId version`, p)