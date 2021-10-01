import tm
import strformat

const version = TM_VERSION(0, 4, 0)

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.exportc, cdecl, dynlib.} =
  let logger = cast[ptr tm_logger_api](reg[].get("tm_logger_api", tm_logger_api_version))
  logger.print(TM_LOG_TYPE_INFO, &"{version=}: " & (if not load: "Unloading old plugin" else: "Hello from Nim!"))