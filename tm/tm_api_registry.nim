template tm_get_api*(reg: ptr tm_api_registry_api, TYPE: untyped): untyped =
  cast[ptr `TYPE`](reg[].get(astToStr(TYPE), `TYPE version`))

template tm_get_optional_api*(reg: ptr tm_api_registry_api, TYPE: untyped): untyped =
  cast[ptr `TYPE`](reg[].get_optional(astToStr(TYPE), `TYPE version`))

template tm_get_api_version*(reg: ptr tm_api_registry_api, TYPE: untyped, VERSION: tm_version_t): untyped =
  cast[ptr `TYPE`](reg[].get(astToStr(TYPE), VERSION))

template tm_set_or_remove_api(reg: ptr tm_api_registry_api, load: bool, TYPE: untyped, impl: untyped): untyped = 
  if load:
    reg[].set(astToStr(TYPE), `TYPE version`, impl, sizeof(impl))
  else:
    reg[].remove(impl)
