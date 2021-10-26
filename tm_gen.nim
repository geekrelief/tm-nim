{.hint[DuplicateModuleImport]:false.}
{.warning[UnusedImport]:false.}

import nimterop/[cimport, paths]
import os, strformat, sugar, strutils, sequtils
import tm / foundation / api_types

proc getHeaders(dir: string): seq[string] =
  result = collect:
    for path in walkDirRec(&"{dir}"):
      echo repr(path)
      if path.endsWith(".h") or path.endsWith(".inl"):
        path

static:
  cDisableCaching()

include "tm_gen_override.nim"
#[
cExclude(tm_headers_dir & "plugins/entity/entity.h")
cExclude(tm_headers_dir & "plugins/entity/transform_component.h")
cExclude(tm_headers_dir & "plugins/the_machinery_shared/component_interfaces/editor_ui_interface.h")

cExclude(tm_headers_dir & "foundation/carray.inl")
cExclude(tm_headers_dir & "foundation/math.inl")
cExclude(tm_headers_dir & "foundation/the_truth.h")

cExclude(tm_headers_dir & "foundation/localizer.h")
]#

cExclude(tm_headers_dir & "foundation/api_types.h")
cIncludeDir(tm_headers_dir)
cDefine("TM_LINKS_FOUNDATION")
cDefine("TM_OS_WINDOWS")
when defined(vcc) or defined(tcc):
  cDefine("_MSC_VER")
when defined(tcc):
  cDefine("TCC")

cImport( flags = "-c -E_ -F_ -G__=_", recurse = true,
  nimFile = "tm/tm_generated.nim", 
  filenames = static(getHeaders(tm_headers_dir))
)
