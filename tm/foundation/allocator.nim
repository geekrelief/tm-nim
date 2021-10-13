macro tm_alloc*(a: ptr tm_allocator_i, init: untyped): untyped =
  let TYPE = init[0]
  result = quote do:
    let pos = instantiationInfo()
    let p = cast[ptr `TYPE`](`a`[].realloc(`a`, cast[pointer](0), 0, sizeof(`TYPE`).uint64, $< pos.filename, pos.line.uint32))
    p[] = `init`
    p

#define tm_alloc_at(a, sz, file, line) (a)->realloc(a, 0, 0, sz, file, line)

#Convenience macro for freeing memory using [[tm_allocator_i]].
#define tm_free(a, p, sz) (a)->realloc(a, p, sz, 0, __FILE__, __LINE__)
template tm_free*(a: ptr tm_allocator_i, p: untyped) =
  let pos = instantiationInfo()
  discard a[].realloc(a, p, sizeof(p[]).uint64, 0, $< pos.filename, pos.line.uint32)

# Convenience macro for reallocating memory using [[tm_allocator_i]].
#define tm_realloc(a, p, old_sz, new_sz) (a)->realloc(a, p, old_sz, new_sz, __FILE__, __LINE__)