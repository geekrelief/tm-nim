proc tmCarrayHeader*(a: pointer): ptr tmCarrayHeaderT {.inline.} =
  cast[ptr tmCarrayHeaderT](cast[ByteAddress](a) - sizeof(tmCarrayHeaderT))

proc tmCarraySize*(a: pointer): uint64 {.inline.} = 
  if a != nil: tmCarrayHeader(a).size else: 0'u64

proc tmCarrayCapacity*(a: pointer): uint64 {.inline.} = 
  if a != nil: tmCarrayHeader(a).capacity else: 0'u64

proc tmCarrayNeedsToGrow*(a: pointer, n: uint64): bool {.inline.} = 
  n > tmCarrayCapacity(a)

proc tmCarrayTempGrow*[T](a: var ptr T, n: uint64, ta: ptr tmTempAllocatorI) {.inline.} =  
  a = cast[ptr T](tmCarrayTempGrowInternal(a, n, sizeof(T).uint64, ta))

proc tmCarrayTempEnsure*[T](a: var ptr T, n: uint64, ta: ptr tmTempAllocatorI) = 
  if tmCarrayNeedsToGrow(a, n): tmCarrayTempGrow(a, n, ta)

proc tmCarrayTempPush*[T](a: var ptr T, item: T, ta: ptr tmTempAllocatorI): ptr T =
  tmCarrayTempEnsure(a, tmCarraySize(a) + 1, ta)
  a[tm_carray_header(a).size.int] = item
  inc tm_carray_header(a).size 
  a[(tm_carray_header(a).size - 1).int].addr

#[
# Returns the header data for `a`.
template tm_carray_header(a) ((tm_carray_header_t *)((uint8_t *)(a) - sizeof(tm_carray_header_t)))

// Returns the number of elements of `a`.
#define tm_carray_size(a) ((a) ? tm_carray_header(a)->size : 0)

// Returns the number of bytes used by `a`'s items (not including header data). This is the amount
// of bytes you need to `memcpy()` to copy all the items.
#define tm_carray_bytes(a) (tm_carray_size(a) * sizeof(*(a)))

// Returns a pointer past the end of `a`. Returns NULL if `a` is not allocated.
#define tm_carray_end(a) ((a) ? (a) + tm_carray_size(a) : 0)

// Returns a pointer to the last element of `a` or `NULL` if `a` is empty.
#define tm_carray_last(a) ((a) ? tm_carray_end(a) - 1 : 0)

// Returns the number of elements allocated for `a`.
#define tm_carray_capacity(a) ((a) ? tm_carray_header(a)->capacity : 0)

// Returns *true* if `a` needs to grow to hold `n` elements.
#define tm_carray_needs_to_grow(a, n) ((n) > tm_carray_capacity(a))

// Pops the last item from `a` and returns it.
#define tm_carray_pop(a) ((a)[--tm_carray_header(a)->size])

// As [[tm_carray_resize()]] but can only shrink the array. Since this won't reallocate memory, you
// don't need to pass an allocator.
#define tm_carray_shrink(a, n) ((a) ? tm_carray_header(a)->size = n : 0)

// tm_allocator_i interface

// Grows the capacity of `a` geometrically to hold at least `n` elements. `a` is updated in-place by
// the macro.
#define tm_carray_grow(a, n, allocator) tm_carray_grow_at(a, n, allocator, __FILE__, __LINE__)

// Ensures that `a` has capacity to to hold at least `n` elements. Grows `a` as needed (geometrically)
// to hold the specified number of elements. `a` is updated in-place by the macro.
#define tm_carray_ensure(a, n, allocator) tm_carray_ensure_at(a, n, allocator, __FILE__, __LINE__)

// Sets the capacity of `a` to exactly `n`. `a` is updated in-place by the macro.
#define tm_carray_set_capacity(a, n, allocator) tm_carray_set_capacity_at(a, n, allocator, __FILE__, __LINE__)

// Pushes `item` to the end of `a`, growing it geometrically if needed. `a` is updated in-place by
// the macro. Returns a pointer to the pushed item in `a`.
#define tm_carray_push(a, item, allocator) tm_carray_push_at(a, item, allocator, __FILE__, __LINE__)

// Inserts `item` at the specified `idx` in `a`, growing it geometrically if needed. `a` is updated
// in-place by the macro. Returns a pointer to the inserted item in `a`.
#define tm_carray_insert(a, idx, item, allocator) (tm_carray_push(a, item, allocator), memmove(a + idx + 1, a + idx, (tm_carray_size(a) - idx - 1) * sizeof(*(a))), a[idx] = (item), a + idx)

// Pushes an array of `n` items to the end of `a`, growing it geometrically if needed. `a` is
// updated in-place by the macro.
#define tm_carray_push_array(a, items, n, allocator) tm_carray_push_array_at(a, items, n, allocator, __FILE__, __LINE__)

// Resizes `a` to `n` elements. If `a` needs to grow, it will grow to exactly `n` elements. Note
// that this growth is not geometric. Use [[tm_carray_resize_geom()]] instead if you want geometric
// growth. `a` is updated in-place by the macro.
#define tm_carray_resize(a, n, allocator) tm_carray_resize_at(a, n, allocator, __FILE__, __LINE__)

// As [[tm_carray_resize()]] but uses geometric growth.
#define tm_carray_resize_geom(a, n, allocator) tm_carray_resize_geom_at(a, n, allocator, __FILE__, __LINE__)

// Creates a carray from the array `ptr` of `n` elements. The carray is allocated using `allocator`.
#define tm_carray_from(ptr, n, allocator) (tm_carray_from_at(ptr, n, allocator, __FILE__, __LINE__))

// Creates an empty carray of type T with `n` zero-initialized elements. The carray is allocated
// using `allocator`.
#define tm_carray_create(T, n, allocator) (tm_carray_create_at(T, n, allocator, __FILE__, __LINE__))

// Frees the memory used by the carray.
#define tm_carray_free(a, allocator) tm_carray_free_at(a, allocator, __FILE__, __LINE__)

// Explicit file and line interface
//
// These macros correspond to the regular macros, but take explicit file and line arguments instead
// of using the current `__FILE__` and `__LINE__`. This can be useful when you want to report memory
// leaks in a higher level system.

#define tm_carray_grow_at(a, n, allocator, file, line) ((*(void **)&(a)) = tm_carray_grow_internal((void *)a, n, sizeof(*(a)), allocator, file, line))

#define tm_carray_ensure_at(a, n, allocator, file, line) (tm_carray_needs_to_grow(a, n) ? tm_carray_grow_at(a, n, allocator, file, line) : 0)

#define tm_carray_set_capacity_at(a, n, allocator, file, line) ((*(void **)&(a)) = tm_carray_set_capacity_internal((void *)a, n, sizeof(*(a)), allocator, file, line))

#define tm_carray_push_at(a, item, allocator, file, line) (tm_carray_ensure_at(a, tm_carray_size(a) + 1, allocator, file, line), (a)[tm_carray_header(a)->size++] = (item), (a) + tm_carray_header(a)->size - 1)

#define tm_carray_insert_at(a, idx, item, allocator, file, line) (tm_carray_push_at(a, item, allocator, file, line), memmove(a + idx + 1, a + idx, (tm_carray_size(a) - idx - 1) * sizeof(*(a))), a[idx] = (item), a + idx)

#define tm_carray_push_array_at(a, items, n, allocator, file, line) ((n) ? ((tm_carray_ensure_at(a, tm_carray_size(a) + n, allocator, file, line), memcpy(a + tm_carray_size(a), items, n * sizeof(*(a))), tm_carray_header(a)->size += n), 0) : 0)

#define tm_carray_resize_at(a, n, allocator, file, line) ((tm_carray_needs_to_grow(a, n) ? tm_carray_set_capacity_at(a, n, allocator, file, line) : 0), (a) ? tm_carray_header(a)->size = n : 0)

#define tm_carray_resize_geom_at(a, n, allocator, file, line) (tm_carray_ensure_at(a, n, allocator, file, line), (a) ? tm_carray_header(a)->size = n : 0)

#define tm_carray_from_at(ptr, n, allocator, file, line) \
    (false ? ptr : tm_carray_create_internal(ptr, n, sizeof(*ptr), allocator, file, line))

#define tm_carray_create_at(T, n, allocator, file, line) \
    ((T *)tm_carray_create_internal(0, n, sizeof(T), allocator, file, line))

#define tm_carray_free_at(a, allocator, file, line) ((*(void **)&(a)) = tm_carray_set_capacity_internal((void *)a, 0, sizeof(*(a)), allocator, file, line))

// tm_temp_allocator_i interface

// As [[tm_carray_grow()]] for arrays allocated using a temp allocator.
#define tm_carray_temp_grow(a, n, ta) ((*(void **)&(a)) = tm_carray_temp_grow_internal((void *)a, n, sizeof(*(a)), ta))

// As [[tm_carray_ensure()]] for arrays allocated using a temp allocator.
#define tm_carray_temp_ensure(a, n, ta) (tm_carray_needs_to_grow(a, n) ? tm_carray_temp_grow(a, n, ta) : 0)

// As [[tm_carray_set_capacity()]] for arrays allocated using a temp allocator.
#define tm_carray_temp_set_capacity(a, n, ta) ((*(void **)&(a)) = tm_carray_temp_set_capacity_internal((void *)a, n, sizeof(*(a)), ta))

// As [[tm_carray_push()]] for arrays allocated using a temp allocator.
#define tm_carray_temp_push(a, item, ta) (tm_carray_temp_ensure(a, tm_carray_size(a) + 1, ta), (a)[tm_carray_header(a)->size++] = (item), (a) + tm_carray_header(a)->size - 1)

// As [[tm_carray_insert()]] for arrays allocated using a temp allocator.
#define tm_carray_temp_insert(a, idx, item, ta) (tm_carray_temp_push(a, item, ta), memmove(a + idx + 1, a + idx, (tm_carray_size(a) - idx - 1) * sizeof(*(a))), a[idx] = (item), a + idx)

#define tm_carray_temp_push_array(a, items, n, ta) ((n) ? ((tm_carray_temp_ensure(a, tm_carray_size(a) + n, ta), memcpy(a + tm_carray_size(a), items, n * sizeof(*(a))), tm_carray_header(a)->size += n), 0) : 0)

// As [[tm_carray_resize()]] for arrays allocated using a temp allocator.
#define tm_carray_temp_resize(a, n, ta) ((tm_carray_needs_to_grow(a, n) ? tm_carray_temp_set_capacity(a, n, ta) : 0), (a) ? tm_carray_header(a)->size = n : 0)

#define tm_carray_temp_resize_geom(a, n, ta) (tm_carray_temp_ensure(a, n, ta), (a) ? tm_carray_header(a)->size = n : 0)
]#