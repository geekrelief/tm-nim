import globals
switch("c") # Compile only. We don't need to generate tm_gen.exe. It doesn't do anything.
switch("cc", cc) # vcc is ok, gcc is ok for compiling, linking with gcc produces errors about duplicate and undefined references
switch("gc", "arc")
switch("include", "./globals.nim")
switch("p", "../gr-nimterop")