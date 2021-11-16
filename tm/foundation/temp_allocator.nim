type TempAllocator* = object
  api*: ptr tm_temp_allocator_api
  o*: tm_temp_allocator_1024_o
  p*: ptr tm_temp_allocator_i

proc `=destroy`*(a: var TempAllocator) =
  if a.p != nil:
    a.api.destroy(a.p)

template init*(a: ptr tm_temp_allocator_api): ptr tm_temp_allocator_i =
  var ta = TempAllocator(api: a)
  ta.p = a.createInBuffer(cast[ptr cchar](ta.o.buffer[0].addr), sizeof(ta.o.buffer).uint64, nil)
  ta.p

#[
// Declares and initializes a regular allocator interface `a` from the temp allocator interface
// `ta`.
#define TM_GET_TEMP_ALLOCATOR_ADAPTER(ta, a) \
    tm_allocator_i a##_object = { 0 };       \
    tm_allocator_i *a = &a##_object;         \
    tm_temp_allocator_api->allocator(a, ta);

// Defines and initializes a temp allocator `ta` as well as a corresponding regular allocator `a`.
// The temp allocator must be shut down with [[TM_SHUTDOWN_TEMP_ALLOCATOR()]]. (The regular allocator
// does not need to be shutdown, since it just forwards its allocations to the temp allocator.)
#define TM_INIT_TEMP_ALLOCATOR_WITH_ADAPTER(ta, a) \
    TM_INIT_TEMP_ALLOCATOR(ta)                     \
    TM_GET_TEMP_ALLOCATOR_ADAPTER(ta, a)
]#