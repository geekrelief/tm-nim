import nimterop/[cimport, paths]
import os, strformat, sugar, strutils, sequtils, macros

const tm_dir = "C:/tm/tm-nim"

static:
  #cDebug()
  cDisableCaching()

proc getHeaders():seq[string] =
  result = collect:
    for i in walkDir(&"{tm_dir}"):
      if i.kind == pcFile and i.path.endsWith(".h"):
        i.path

cIncludeDir(r".")
cDefine("TM_LINKS_FOUNDATION")
cDefine("_MSC_VER")
cDefine("TM_OS_WINDOWS")

cOverride:
  template TM_VERSION*(Major, Minor, Patch): untyped  =
    tm_version_t(major: Major, minor: Minor, patch: Patch)


#cImport(filenames = static(getHeaders()), nimFile = "tm.nim")
cImport( flags = "-E_ -F_ -G__=_", recurse = true,
  nimFile = "tm.nim", 
  filenames = static(getHeaders())
)
