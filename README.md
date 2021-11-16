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

## Using Tiny C Compiler / TCC ##
- Prereqs
  - Copy `tcc\intrin.h` to your tcc include folder for win32. It's used by `math.inl`.
  - Run `nimble removePragmaOnce` script modifies the headers to work with tcc. TCC, doesn't support the non-standard `#pragma once`. 
  - `foundation/api_types.h` needs modification to check for `TCC` so it defines `TM_DISABLE_PADDING_WARNINGS` and `TM_RESTORE_PADDING_WARNINGS` as nothing. Inside of the `if defined(TM_OS_WINDOWS)` check add:
```c
  #if defined(TCC)
  #define TM_DISABLE_PADDING_WARNINGS
  #define TM_RESTORE_PADDING_WARNINGS
  #else
``` 
and add `#endif` after the define for `TM_RESTORE_PADDING_WARNINGS`.

- Modify tm.nimble to set the compiler to `tcc`
- Run `nimble gen` again to regenerate `tm/tm_generated.nim`, then build your plugin

---
## Adding New Headers
- Copy the headers you need from The Machinery to the `headers` dir, or you can try wrapping everything by modifying `globals.nim` to point to The Machinery SDK headers dir.
- Run `nimble gen`

---
## Common Issues / Workarounds
The following is a list of common issues and workarounds when adding new headers for the generator.

### Opaque data: pointers ot `_o` or `_t` types.
If you get an `Error: undeclared identifier`  about an opaque type, ending in `_o` or `_t`, override it in `tm_gen_override.nim`, and run `nimble gen` again.

TM uses a lot of opaque ptrs, typedefs with _o suffix, and other types that are passed between API calls. If all we have to do is pass these along between calls we can define an empty `object` for them. If TM, expects us to define our own data through an opaque type, we can override their definition and add an `{.inheritable.}` pragma to them.  This makes it possible to create subtypes for opaque types to interop between Nim and C. For example:
```nim
# in tm_gen_override.nim
type 
  tm_simulation_state_o* {.inheritable.} = object
# in our plugin
type
  # we can cast 
  sim_state_o = object of tm_simulation_state_o
    # custom data
```

### Unions
To handle unions, flatten the structure/union by bringing all the fields into the object. Rely on `importc` to handle all the fields correctly.

### TM_INHERITS
TM defines some `struct` types with another `struct` declaration at the top using TM_INHERITS. The workaround is to copy all fields from the referenced `struct` into the current `struct`.

### Proc Type issues
The Machinery uses lots of function pointers and callbacks. There's a custom pragma `tmType` you can attach to a proc to make it easier to interact with the api. Without it you need to cast the proc.

### Pointers and arrays
Since TM is written in C we're going to have to manipulate lots of `UncheckedArray`s and `ptr`/`pointer`s. The `ptr_math` package is included to make things easier to work with. I've also added iterators for `m/items`, `m/pairs` that work with `UncheckedArray` and `ptr T`.
### Stylistic Duplicate
  - On Identifier 'Foo' is a stylistic duplicate of identifier 'foo', 'use cPlugin:onSymbol()', modify `tm_gen_onsymbol.nim` to rename the identifier.
### Invalid Pragma Error
In tm_gen.nim, cImport uses `recurse = true`, this preprocesses the headers, flattening them, so we lose track of which file a type actually comes from.  For example, if `a.h` includes `b.h` and `b.h` defines `foo`, when nimterop creates the header pragma `foo` will be marked as coming from `a.h` like `impaHdr`.  If you override something, and reuse a nimterop custom `header` pragma you might run into an `invalid pragma` error.  Replace the nimterop custom `header` pragma, with a regular `header` pragma that points to the file where the definition exists.

### Include Dependencies in `.inl`
You may run into a situation where after a header file is included, nimterop produces a binding that cannot compile properly. You may see a strange error even though the C code is valid. This may be because a `.inl` file is including other files. The `.inl` may not be including a header file it has a dependency on, so it can't be preprocessed properly. For example, a `define` symbol might be used from `api_types.h`, but it is not included in the `.inl`.

### TCC and pragma once ###
When adding new headers, run `nimble removePragmaOnce` because TCC does not recognize `#pragma once`.