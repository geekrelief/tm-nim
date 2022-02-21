include tm
import strformat

const version = TM_VERSION(0, 2, 0)

proc tm_load_plugin(reg: ptr tm_api_registry_api, load: bool) {.callback.} =
  if load: 
    NimMain()

  let logger = reg.get_api tm_logger_api

  logger.print(TM_LOG_TYPE_INFO, cstring(&"{version=}: " & (if not load: "Unloading old plugin" else: "Hello from Minimal!")))

  if load:
    var id = tt_id(3, 2, 1)
    logger.print(TM_LOG_TYPE_INFO, cstring &"{id.`type`=} {id.generation=} {id.index=} {tm_tt_type_t(id).u64=}")
    logger.print(TM_LOG_TYPE_INFO, cstring &"{TM_STATIC_HASH(\"minimal\").uint64 = :#x}")