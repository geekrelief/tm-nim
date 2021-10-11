# tm-nim is a WIP bindings generator for The Machinery. #

## Generate the bindings ##
- Modify `globals.nim` to choose your compiler and the location of The Machinery headers. (gcc, vcc, tcc have been tested on Windows.)
- In this repo a `headers` directory is included for wrapping a subset of the headers.
- Run `nimble build` to generate the bindings.

## Using the bindings ##
- Compile `nim c minimal.nim` for a minimal sample.
- Copy `build/tm_minimal.dll` to your TM `plugins` folder.