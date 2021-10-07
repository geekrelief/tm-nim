{.hint[DuplicateModuleImport]:false.}
{.warning[UnusedImport]:false.}

import nimterop/[cimport, paths]
import os, strformat, sugar, strutils, sequtils, macros

import globals

static:
  #cDebug()
  cDisableCaching()

proc getHeaders():seq[string] =
  result = collect:
    for i in walkDir(&"{tm_dir}"):
      if i.kind == pcFile and i.path.endsWith(".h"):
        i.path

cExclude("api_types.h")
cIncludeDir(r".")
cDefine("TM_LINKS_FOUNDATION")
cDefine("_MSC_VER")
cDefine("TM_OS_WINDOWS")

#cImport(filenames = static(getHeaders()), nimFile = "tm.nim")
cImport( flags = "-E_ -F_ -G__=_", recurse = true,
  nimFile = "tm_generated.nim", 
  filenames = static(getHeaders())
)
