{.hint[DuplicateModuleImport]:false.}
{.warning[UnusedImport]:false.}

import nimterop/[cimport, paths]
import os, strformat, sugar, strutils, sequtils, macros
import tm / api_types

static:
  cDisableCaching()

proc getHeaders():seq[string] =
  result = collect:
    for i in walkDir(&"{tm_dir}"):
      if i.kind == pcFile and i.path.endsWith(".h"):
        i.path

cExclude(tm_dir & "api_types.h")
cDefine("TM_LINKS_FOUNDATION")
when defined(vcc) or defined(tcc):
  cDefine("_MSC_VER")
cDefine("TM_OS_WINDOWS")

cImport( flags = "-E_ -F_ -G__=_", recurse = true,
  nimFile = "tm/tm_generated.nim", 
  filenames = static(getHeaders())
)
