#pragma once

#include "allocator.h"
#include "api_types.h"
#include "macros.h"
#include "temp_allocator.h"

#include <string.h>

// This file implements a growing array in C. It's based on Sean Barrett's [stretchy
// buffer](https://github.com/nothings/stb/blob/master/stretchy_buffer.h) implementation. An array
// is represented just by a regular pointer to the array object type:
//
// ~~~c
// uint32_t *a = 0;
// ~~~
//
// Size and capacity of the array are stored in a header before the array pointer itself. The case
// where the array is `NULL` is handled correctly. Note that the functions here will modify the
// array pointer itself, so if you pass the carray to a function for modification, you must pass a
// pointer to it.
//
// You can also use static memory for the initial setup of an array, as long as you set the capacity
// to zero. This signals that the memory is statically allocated and should not be freed:
//
// ~~~c
// // b[-1] is the size and b[-2] the capacity.
// uint64_t b_data[] = {0, 3, 1, 2, 3};
// uint64_t *b = b_data + 2;
// ~~~

// Header placed in front of the array data.
typedef struct tm_carray_header_t
{
    uint64_t capacity;
    uint64_t size;
} tm_carray_header_t;

// Generic carray functions

// Returns the header data for `a`.
#define tm_carray_header(a) ((tm_carray_header_t *)((uint8_t *)(a) - sizeof(tm_carray_header_t)))

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

// tm_docgen off

static inline void *tm_carray_set_capacity_internal(void *arr, uint64_t new_capacity, uint64_t item_size,
    struct tm_allocator_i *allocator, const char *file, uint32_t line)
{
    uint8_t *p = arr ? (uint8_t *)tm_carray_header(arr) : 0;
    const uint64_t extra = sizeof(tm_carray_header_t);
    const uint64_t size = tm_carray_size(arr);
    const uint64_t bytes_before = arr ? item_size * tm_carray_capacity(arr) + extra : 0;
    const uint64_t bytes_after = new_capacity ? item_size * new_capacity + extra : 0;
    if (p && !tm_carray_capacity(arr)) {
        // Static allocation
        uint8_t *old_p = p;
        p = (uint8_t *)allocator->realloc(allocator, 0, 0, bytes_after, file, line);
        const uint64_t static_bytes = item_size * size + extra;
        const uint64_t to_copy = static_bytes > bytes_after ? bytes_after : static_bytes;
        memcpy(p, old_p, to_copy);
    } else
        p = (uint8_t *)allocator->realloc(allocator, p, bytes_before, bytes_after, file, line);
    void *new_a = p ? p + extra : p;
    if (new_a) {
        tm_carray_header(new_a)->size = size;
        tm_carray_header(new_a)->capacity = new_capacity;
    }
    return new_a;
}

static inline void *tm_carray_grow_internal(void *arr, uint64_t to_at_least, uint64_t item_size,
    struct tm_allocator_i *allocator, const char *file, uint32_t line)
{
    const uint64_t capacity = arr ? tm_carray_capacity(arr) : 0;
    if (capacity >= to_at_least)
        return arr;
    const uint64_t min_new_capacity = capacity ? capacity * 2 : 16;
    const uint64_t new_capacity = tm_max(min_new_capacity, to_at_least);
    return tm_carray_set_capacity_internal(arr, new_capacity, item_size, allocator, file, line);
}

static inline void *tm_carray_create_internal(const void *ptr, uint64_t size, uint64_t item_size,
    struct tm_allocator_i *allocator, const char *file, uint32_t line)
{
    void *arr = tm_carray_set_capacity_internal(0, size, item_size, allocator, file, line);
    tm_carray_header(arr)->size = size;
    if (ptr)
        memcpy(arr, ptr, size * item_size);
    else
        memset(arr, 0, size * item_size);
    return arr;
}

static inline void *tm_carray_temp_set_capacity_internal(void *arr, uint64_t new_capacity, uint64_t item_size,
    struct tm_temp_allocator_i *ta)
{
    uint8_t *p = arr ? (uint8_t *)tm_carray_header(arr) : 0;
    uint64_t extra = sizeof(tm_carray_header_t);
    const uint64_t size = tm_carray_size(arr);
    const uint64_t bytes_before = arr ? item_size * tm_carray_capacity(arr) + extra : 0;
    const uint64_t bytes_after = new_capacity ? item_size * new_capacity + extra : 0;
    if (p && !tm_carray_capacity(arr)) {
        // Static allocation
        uint8_t *old_p = p;
        p = (uint8_t *)ta->realloc(ta->inst, 0, 0, bytes_after);
        const uint64_t static_bytes = item_size * size + extra;
        const uint64_t to_copy = static_bytes > bytes_after ? bytes_after : static_bytes;
        memcpy(p, old_p, to_copy);
    } else
        p = (uint8_t *)ta->realloc(ta->inst, p, bytes_before, bytes_after);
    void *new_a = p ? p + extra : p;
    if (new_a) {
        tm_carray_header(new_a)->size = size;
        tm_carray_header(new_a)->capacity = new_capacity;
    }
    return new_a;
}

static inline void *tm_carray_temp_grow_internal(void *arr, uint64_t to_at_least, uint64_t item_size,
    struct tm_temp_allocator_i *ta)
{
    const uint64_t capacity = arr ? tm_carray_capacity(arr) : 0;
    const uint64_t min_new_capacity = capacity ? capacity * 2 : 16;
    const uint64_t new_capacity = tm_max(min_new_capacity, to_at_least);
    return tm_carray_temp_set_capacity_internal(arr, new_capacity, item_size, ta);
}
