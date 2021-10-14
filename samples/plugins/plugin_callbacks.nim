include tm

proc tm_load_plugin(reg: ptr tm_api_registry, load: bool) {.callback.} =
  if load:
    NimMain()
