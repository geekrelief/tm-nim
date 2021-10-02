switch("c") # Compile only. We don't need to generate tm_gen.exe. It doesn't do anything.
let cc = "tcc"
switch("cc", "tcc") # vcc is ok, gcc is ok for compiling, linking with gcc produces errors about duplicate and undefined references
switch("gc", "arc")
switch("p", "../gr-nimterop")