macro alloc*(a: ptr tm_allocator_i, init: typed): untyped =
  ## Allocates and initializes a structure to pass to TM
  ## Ex: 
  ## result = args.allocator.alloc(simulation_state_o(
  ##   allocator: args.allocator,
  ##   tt: args.tt,
  ##   entity_ctx: args.entity_ctx,
  ##   simulation_ctx: args.simulation_ctx))
  #echo "init type " & getTypeInst(init).astGenRepr
  
  result = genAst(a, TYPE = getTypeInst(init), init):
    let i = instantiationInfo()
    let p = cast[ptr TYPE](a.realloc(a, cast[pointer](0), 0, sizeu64(TYPE), cstring(i.filename), i.line.uint32))
    p[] = init
    p

#define tm_alloc_at(a, sz, file, line) (a)->realloc(a, 0, 0, sz, file, line)

template free*(a: ptr tm_allocator_i, p: untyped) =
  ## Free memory allocated by alloc.
  ## Ex: 
  ##  var s = cast[ptr simulation_state_o](state)
  ##  s.allocator.free(s)
  let i = instantiationInfo()
  discard a[].realloc(a, p, sizeu64(p[]), 0, cstring(i.filename), i.line.uint32)

# Convenience macro for reallocating memory using [[tm_allocator_i]].
#define tm_realloc(a, p, old_sz, new_sz) (a)->realloc(a, p, old_sz, new_sz, __FILE__, __LINE__)