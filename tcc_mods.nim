# Modifies the headers for use with TinyC


import std / [os, strutils, strformat]

const headerDir = "headers"

# #pragma once prevents repeated includes of the same header.
# Below pragma once is replaced with the idiomatic #ifndef #define #endif
for p in walkDirRec(headerDir):
  var (path, name, ext) = p.splitFile
  var flag = (path.splitPath[1] & "_" & name).toUpperAscii
  var data = readFile(p)
  if data.find("#pragma once") != -1:
    data = data.replace("#pragma once", &"#ifndef {flag}\p#define {flag}")
    data &= "\n\n#endif"
    writeFile(p, data)

# Inside foundation/api_types.h, specify nothing for TM_DISABLE_PADDING_WARNINGS and TM_RESTORE_PADDING_WARNINGS for TCC
# We assume there are no other elif clauses between TM_OS_WINDOWS and __clang__
const 
  apiTypesPath = headerDir & "/foundation/api_types.h"
  paddingsStart = "#if defined(TM_OS_WINDOWS)"
  paddingsEnd = "#elif defined(__clang__)"
  paddingsTcc = "#if defined(TCC)"

var data = readFile(apiTypesPath)
var startDx = data.find(paddingsStart)
var endDx = data.find(paddingsEnd)
doAssert startDx > -1, &"Error in foundation/api_types.h: Could not find paddings start {startDx = }"
doAssert endDx > -1, &"Error in foundation/api_types.h: Could not find paddings end {endDx = }"
var tccDx = data.find(paddingsTcc, startDx)
if tccDx > startDx and tccDx > -1 and tccDx < endDx:
  # if TCC is defined inside the padding section skip
  discard
else: 
  # no TCC or it's defined outside the padding section
  var before = data[0..<(startDx + paddingsStart.len)]
  var middle = data[(startDx + paddingsStart.len)..<endDx]
  var after = data[endDx .. ^1]
  
  var output = before & "\n\n" & paddingsTCC & "\n#define TM_DISABLE_PADDING_WARNINGS\n#define TM_RESTORE_PADDING_WARNINGS\n#else\n" & middle & "#endif\n" & after
  writeFile(apiTypesPath, output)