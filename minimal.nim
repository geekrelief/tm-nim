import tm
import strformat

const version = TM_VERSION(0, 7, 0)

proc NimMain() {.dynlib, exportc, cdecl, importc.}
proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.exportc, cdecl, dynlib.} =
  if load: 
    NimMain()

  let logger = cast[ptr tm_logger_api](reg[].get("tm_logger_api", tm_logger_api_version))
  if load:
    logger.print(TM_LOG_TYPE_INFO, "...TCC compiled")

  
  logger.print(TM_LOG_TYPE_INFO, cstring &"{version=}: " & (if not load: "Unloading old plugin" else: "Hello from Nim!"))