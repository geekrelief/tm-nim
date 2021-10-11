include tm
import strformat

const version = TM_VERSION(0, 1, 0)

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  let logger = reg.tm_get_api(tm_logger_api)

  if load:
    logger.print(TM_LOG_TYPE_INFO, "...TCC compiled")
  
  logger.print(TM_LOG_TYPE_INFO, cstring &"{version=}: " & (if not load: "Unloading old plugin" else: "Hello from Nim!"))

  var id = tt_id(3, 2, 1)
  logger.print(Tmlogtypeinfo, $$ &"{id.`type`=} {id.generation=} {id.index=} {tm_tt_type_t(id).u64=}")