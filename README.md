# tm-nim is a WIP binding generator for The Machinery. #

Tested: With vcc and tcc on Windows, and The Machinery master branch.
## Generate the binding ##
- Modify `tm.nimble` configuration variables section for your needs.
- In this repo a `headers` directory is included for wrapping a subset of the headers.
- Run `nimble gen` to generate the bindings.

## Using the binding ##
- Run `nimble minimal` to build a minimal sample.
- Create a plugin scaffold with `nimble new`

## Building your plugin ##
- Add a new task to `tm.nimble` to build your plugin. See the examples at the bottom of the file.

## Using TinyCC / tcc ##
- Prereqs
  - Copy `tcc\intrin.h` to your tcc include folder for win32. It's used by `math.inl`.
  - `nim r remove_pragma_once.nim` script modifies the headers to work with tcc.  tcc doesn't support the non-standard `#pragma once`. 
  - `foundation/api_types.h` is modified to check for `TCC` so it defines `TM_DISABLE_PADDING_WARNINGS` and `TM_RESTORE_PADDING_WARNINGS` with nothing. They're used in `math.inl`.
- Modify tm.nimble to set the compiler to `tcc`
- Run `nimble gen` again to regenerate `tm/tm_generated.nim`, then build your plugin

## Adding new headers to the binding ##
- Copy the headers you need from The Machinery to the `headers` dir, or you can try wrapping everything by modifying `globals.nim` to point to The Machinery SDK headers dir.
- Run `nimble gen`
  - If you get an `Error: undeclared identifier ...` about an opaque type, ending in `_o`, override it in `tm_gen_override.nim`, and run `nimble gen` again.
  - Union: Create a new type with all the fields (flattened). With `importc` Nim will import them correctly.

## Proc Type issues ##
The Machinery uses lots of function pointers and callbacks. There's a custom pragma `tmType` you can attach to a proc to make it easier to interact with the api. Without it you need to cast the proc.

## Pointers and arrays ##
Since TM is written in C we're going to have to manipulate lots of `UncheckedArray`s and `ptr`/`pointer`s. The `ptr_math` package is included to make things easier to work with. I've also added iterators for `m/items`, `m/pairs` that work with `UncheckedArray` and `ptr T`.