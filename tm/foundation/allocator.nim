# Convenience macro for allocating memory using [[tm_allocator_i]].
#define tm_alloc(a, sz) (a)->realloc(a, 0, 0, sz, __FILE__, __LINE__)
template tm_alloc*(a: ptr tm_allocator_i, sz: uint64): untyped =
  let pos = instantiationInfo()
  a[].realloc(a, cast[pointer](0), 0'u64, sz, $< pos.filename, pos.line.uint32)

#define tm_alloc_at(a, sz, file, line) (a)->realloc(a, 0, 0, sz, file, line)

#Convenience macro for freeing memory using [[tm_allocator_i]].
#define tm_free(a, p, sz) (a)->realloc(a, p, sz, 0, __FILE__, __LINE__)
template tm_free*(a: ptr tm_allocator_i, p: pointer, sz: uint64) =
  let pos = instantiationInfo()
  discard a[].realloc(a, p, sz, 0'u64, $< pos.filename, pos.line.uint32)

# Convenience macro for reallocating memory using [[tm_allocator_i]].
#define tm_realloc(a, p, old_sz, new_sz) (a)->realloc(a, p, old_sz, new_sz, __FILE__, __LINE__)