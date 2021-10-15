# tm-nim is a WIP binding generator for The Machinery. #

Note: Only works with vcc on Windows.
## Generate the binding ##
- In this repo a `headers` directory is included for wrapping a subset of the headers.
- Modify `tm_gen.nim` and `tm_gen_override.nim` as needed.
- Run `nimble build` to generate the bindings.

## Using the binding ##
- Run `nimble minimal` to build a minimal sample.
- Copy `build/tm_minimal.dll` to your TM `plugins` folder.
- Create a plugin scaffold with `nimble new`

## Building your plugin ##
- Add a new task to `tm.nimble` to build your plugin. See the examples at the bottom of the file.

## Adding new headers to the binding ##
- The generator doesn't wrap all of The Machinery.
- Copy the headers from The Machinery to the `headers` dir, or you can try wrapping everything by modifying `globals.nim` to The Machinery headers.
- Run `nimble gen`
  - If you get an `Error: undeclared identifier ...` about an opaque type, ending in `_o`, override it in `tm_gen_override.nim`, and run `nimble gen` again.
  - opaque types from The Machinery that are meant to be defined by the user should be object types marked with `{.inheritable.}` and with the subtypes defined in your plugin code.