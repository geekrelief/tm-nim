{.hint[DuplicateModuleImport]:false.}
{.warning[UnusedImport]:false.}

import nimterop/[cimport, paths]
import os, strformat, sugar, strutils, sequtils, macros
import tm / foundation / api_types

proc getHeaders(dir: string): seq[string] =
  result = collect:
    for path in walkDirRec(&"{dir}"):
      echo repr(path)
      if path.endsWith(".h"):
        path

static:
  cDisableCaching()

include "override.nim"

cExclude(tm_headers_dir & "foundation/api_types.h")
cIncludeDir(tm_headers_dir)
cDefine("TM_LINKS_FOUNDATION")
cDefine("_MSC_VER") # only works with vcc, other compilers not supported
cDefine("TM_OS_WINDOWS")

cImport( flags = "-c -E_ -F_ -G__=_", recurse = true,
  nimFile = "tm/tm_generated.nim", 
  filenames = static(getHeaders(tm_headers_dir))
)
