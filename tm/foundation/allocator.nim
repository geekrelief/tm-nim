macro tmAlloc*(a: ptr tmAllocatorI, init: untyped): untyped =
  result = genAst(a, TYPE = init[0], init):
    let i = instantiationInfo()
    let p = cast[ptr TYPE](a.realloc(a, cast[pointer](0), 0, sizeof(TYPE).uint64, cstring(i.filename), i.line.uint32))
    p[] = init
    p

#define tm_alloc_at(a, sz, file, line) (a)->realloc(a, 0, 0, sz, file, line)

#Convenience macro for freeing memory using [[tm_allocator_i]].
template tmFree*(a: ptr tm_allocator_i, p: untyped) =
  let i = instantiationInfo()
  discard a[].realloc(a, p, sizeof(p[]).uint64, 0, cstring(i.filename), i.line.uint32)

# Convenience macro for reallocating memory using [[tm_allocator_i]].
#define tm_realloc(a, p, old_sz, new_sz) (a)->realloc(a, p, old_sz, new_sz, __FILE__, __LINE__)