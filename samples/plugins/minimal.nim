import tm
import strformat

const version = TmVersion(0, 2, 0)

proc tmLoadPlugin(reg: ptr tmApiRegistryApi, load: bool) {.callback.} =
  if load: 
    NimMain()

  let logger = reg.tmGetApi tmLoggerApi

  logger.print(TmLogTypeInfo, cstring(&"{version=}: " & (if not load: "Unloading old plugin" else: "Hello from Minimal!")))

  if load:
    var id = ttId(3, 2, 1)
    logger.print(TmLogTypeInfo, cstring &"{id.`type`=} {id.generation=} {id.index=} {tmTtTypeT(id).u64=}")
    logger.print(TmLogTypeInfo, cstring &"{TmStaticHash(\"minimal\").uint64 = :#x}")