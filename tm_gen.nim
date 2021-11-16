{.hint[DuplicateModuleImport]:false.}
{.warning[UnusedImport]:false.}

import nimterop/[cimport, paths]
import globals, std / [os, osproc, sugar, strformat, strutils, sequtils, streams, re, algorithm, sets]
import tm / foundation / api_types

static:
  cDisableCaching()

include "tm_gen_onsymbol.nim"
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


var filenames {.compileTime.}: seq[string]
static:
  var headers = staticRead("tm_header_deps.txt").split("\n")
  for h in headers:
    filenames.add h

cImport( flags = "--enumNotDistinct --enumBaseTypeStr:cuint --noComments", verbose = true, recurse = true,
  nimFile = "tm/tm_generated.nim", 
  filenames = filenames)