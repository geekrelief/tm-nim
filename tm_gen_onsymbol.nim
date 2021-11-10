cPlugin:
  import strutils
  proc onSymbol*(sym: var Symbol) {.exportc, dynlib.} =
    sym.name = sym.name.replace("__", "_").strip(chars = {'_'})
    sym.name = case sym.name:
      of "LZ4_versionNumber": #foundation/lz4_external.h: collides with define LZ4_VERSION_NUMBER
        "LZ4_versionNumber_f"
      else:
        sym.name