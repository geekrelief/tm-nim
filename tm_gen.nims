import globals
switch("c") # Compile only. We don't need to generate tm_gen.exe. It doesn't do anything.
switch("cc", "vcc")
switch("gc", "arc")
switch("include", "./globals.nim")

when defined(dev):
  switch("path", "../gr-nimterop")