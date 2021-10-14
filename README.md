# tm-nim is a WIP bindings generator for The Machinery. #

Note: Only works with vcc on Windows.
## Generate the bindings ##
- Modify `globals.nim` to set the location of The Machinery headers.
- In this repo a `headers` directory is included for wrapping a subset of the headers.
- Modify `tm_gen.nim` and `tm_gen_override.nim` as needed.
- Run `nimble build` to generate the bindings.

## Using the bindings ##
- Run `nimble minimal` to build a minimal sample.
- Copy `build/tm_minimal.dll` to your TM `plugins` folder.

## Adding new headers to the binding **
- Copy the header file to the headers dir.
- Run `nimble gen`
  - If you get an `Error: undeclared identifier ...` about an opaque type, ending in `_o`, override it in `tm_gen_override.nim`, and run `nimble gen` again.