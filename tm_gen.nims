import globals
--compileOnly:on # We don't need to generate tm_gen.exe. It doesn't do anything.
--"include":"./globals.nim"

when defined(dev):
  --path:"../gr-nimterop"