include tm
import strformat

const version = TM_VERSION(0, 7, 0)

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  let logger = cast[ptr tm_logger_api](reg[].get("tm_logger_api", tm_logger_api_version))
  if load:
    logger.print(TM_LOG_TYPE_INFO, "...TCC compiled")

  
  logger.print(TM_LOG_TYPE_INFO, cstring &"{version=}: " & (if not load: "Unloading old plugin" else: "Hello from Nim!"))

  var tmTtIdT = TtId(1, 2, 3)
  logger.print(Tmlogtypeinfo, $$ &"{tmTtIdT.generation=}")