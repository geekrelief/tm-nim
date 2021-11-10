# #pragma once is a non standard directive to prevent multiple includes of a header
# This script replaces it, and wraps the header with the indiomatic #ifndef #define #endif
# This also makes it possible to use with TCC.
import std / [os, strutils, strformat]

const headerDir = "headers"

for p in walkDirRec(headerDir):
  var (path, name, ext) = p.splitFile
  var flag = (path.splitPath[1] & "_" & name).toUpperAscii
  var data = readFile(p)
  if data.find("#pragma once") != -1:
    data = data.replace("#pragma once", &"#ifndef {flag}\p#define {flag}")
    data &= "\n\n#endif"
    writeFile(p, data)