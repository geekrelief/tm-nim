#pragma once

#include "api_types.h"

struct tm_allocator_i;
struct tm_temp_allocator_o;

// Provides a system for temporary memory allocations. I.e., short-lived memory allocations that are
// automatically freed when the allocator is destroyed. Temp allocators typically use a pointer bump
// allocator to allocate memory from one or more big memory blocks and then free the entire block
// when the allocator is destroyed.

// Interface for temp allocators. Temp allocators are used for short-term temporary memory
// allocations. Memory allocated through a temporary allocator doesn't need to be explicitly freed.
// It is automatically destroyed when the allocator is destroyed.
//
// Temp allocators don't use memory tracking, since the memory they allocate is assumed to be
// short-lived.
typedef struct tm_temp_allocator_i
{
    // User data for this temp allocator.
    struct tm_temp_allocator_o *inst;

    // Allocates memory through the temporary allocator. This function works as the [[realloc()]]
    // function in [[tm_allocator_i]] with two exceptions:
    //
    // * Memory does not need to be explicitly freed, it is automatically freed when the temp
    //   allocator is destroyed.
    // * Memory tracking (file, line, scope) is not used, since the memory is short-lived anyway.
    void *(*realloc)(struct tm_temp_allocator_o *inst, void *ptr, uint64_t old_size, uint64_t new_size);
} tm_temp_allocator_i;

// User data for the [[tm_temp_allocator_1024_o]] temp allocator.
//
// This allocator uses 1024 bytes of stack storage (`buffer`) to use for the temp allocator
// structure and allocations. It first tries to allocate from this buffer and when it is exhausted,
// it will use the backing allocator.
//
// Note that this means that if you are allocating < 1K your allocations will not touch the backing
// allocator. You get the same performance as if you were just using local variables.
//
// To use this allocator, you declare a local variable of type [[tm_temp_allocator_1024_o]] and then
// use [[tm_temp_allocator_api->create_in_buffer()]] to create the temp allocator and
// [[tm_temp_allocator_api->destroy()]] to destroy it.
//
// Typically, you would never do this directly though, as the [[TM_INIT_TEMP_ALLOCATOR()]] and
// [[TM_SHUTDOWN_TEMP_ALLOCATOR()]] macros do it for you.
typedef struct tm_temp_allocator_1024_o
{
    // Buffer used to serve the initial memory requests.
    char buffer[1024];
} tm_temp_allocator_1024_o;

// Statistics for the temp allocator.
typedef struct tm_temp_allocator_statistics_t
{
    // Number of blocks allocated for the temp allocator (since this counter was externally reset).
    uint64_t temp_allocation_blocks;

    // Number of bytes allocated for the temp allocator (since this counter was externally reset).
    uint64_t temp_allocation_bytes;

    // Number of currently live frame allocation blocks.
    uint64_t frame_allocation_blocks;

    // Number of bytes allocated for the frame allocator (since this counter was externally reset).
    uint64_t frame_allocation_bytes;
} tm_temp_allocator_statistics_t;

// API for temp allocators.
struct tm_temp_allocator_api
{
    // Creates a [[tm_temp_allocator_i]] that doesn't use any stack space. Instead, all the memory
    // is allocated using the `backing` allocator. This can be used in situations where the temp
    // allocator needs to live longer than the current stack.
    //
    // If `backing` is NULL, the default backing allocator will be used.
    tm_temp_allocator_i *(*create)(struct tm_allocator_i *backing);

    // As [[create()]] but uses the static `buffer` of `size` to hold both the allocator data and
    // the initial allocation.
    tm_temp_allocator_i *(*create_in_buffer)(char *buffer, uint64_t size, struct tm_allocator_i *backing);

    // Destroys a temp allocator created with [[create()]].
    void (*destroy)(struct tm_temp_allocator_i *ta);

    // Creates a regular allocator `a` from a temp allocator `ta`.
    //
    // This function provides "glue" code for adapting the temp allocator interface
    // [[tm_temp_allocator_i]] to the regular allocator interface [[tm_allocator_i]] so that you can use
    // the temp allocator with systems that expect a regular allocator.
    //
    // Typically, you would not call this function directly, but instead use the
    // [[TM_INIT_TEMP_ALLOCATOR_WITH_ADAPTER()]] macro to create both a temporary allocator and a
    // regular allocator interface for it.
    void (*allocator)(struct tm_allocator_i *a, tm_temp_allocator_i *ta);

    // Allocates memory for the current "frame".
    //
    // Temp allocated memory is automatically freed when the temp allocator is destroyed (goes out
    // of scope). Frame allocated memory is automatically freed at the end of the current frame
    // (when [[tick_frame()]] is called).
    //
    // Frame memory is even simpler to use than temp memory, because you don't have to keep track of
    // the allocator's lifetime, but it has a higher cost, because the allocations live longer (for
    // a whole frame, rather than just the current scope). A typical use case is for small UI
    // strings that gets rendered later in the frame. Since it's just a small allocation, having it
    // live for the duration of the frame is not a huge problem and since the data is used later
    // than when it's generated, using a scoped temporary allocator doesn't work very well.
    //
    // !!! NOTE
    //     If you use this function anywhere in your code, you must call [[tick_frame()]]
    //     regularly in order to eventually free the frame allocated memory.
    void *(*frame_alloc)(uint64_t size);

    // Returns an allocator that uses [[frame_alloc()]] to service allocation requests.
    struct tm_allocator_i *(*frame_allocator)(void);

    // Ticks a "frame" so that memory allocated by [[frame_alloc()]] can be freed.
    void (*tick_frame)(void);

    // As C [[printf()]], but prints to a buffer allocated by the temporary allocator `ta` and returns
    // the result.
    char *(*printf)(tm_temp_allocator_i *ta, const char *format, ...);

    // `va_list` version of [[printf()]].
    char *(*vprintf)(tm_temp_allocator_i *ta, const char *format, va_list args);

    // As C [[printf()]], but prints to a buffer allocated by the frame allocator and returns the
    // result.
    char *(*frame_printf)(const char *format, ...);

    // `va_list` version of [[frame_printf()]].
    char *(*frame_vprintf)(const char *format, va_list args);

    // Holds statistics for the temp allocator system.
    tm_temp_allocator_statistics_t *statistics;
};

#define tm_temp_allocator_api_version TM_VERSION(1, 0, 0)

// Defines a [[tm_temp_allocator_1024_o]] variable and initialies it with
// [[tm_temp_allocator_api->create_in_buffer()]]. You typically use this together with
// [[TM_SHUTDOWN_TEMP_ALLOCATOR()]] to create a scoped temporary allocator:
//
// ~~~c
// {
//     TM_INIT_TEMP_ALLOCATOR(ta);
//     // Use `ta` here.
//     TM_SHUTDOWN_TEMP_ALLOCATOR(ta);
// }
// ~~~
//
// !!! NOTE
//     If you forget the [[TM_SHUTDOWN_TEMP_ALLOCATOR()]], memory will be leaked. There is some
//     protection against this in the `INIT` macro. It defines a variable with a unique name that
//     is used in the `SHUTDOWN` macro, so if you forget `SHUTDOWN` you will get a warning about
//     an unused variable.
#define TM_INIT_TEMP_ALLOCATOR(ta_name)                       \
    uint32_t ta_name##_TM_SHUTDOWN_TEMP_ALLOCATOR_is_missing; \
    tm_temp_allocator_1024_o ta_name##_object;                \
    tm_temp_allocator_i *ta_name = tm_temp_allocator_api->create_in_buffer(ta_name##_object.buffer, sizeof(ta_name##_object.buffer), 0);

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

// Shuts down a temp allocator created by [[TM_INIT_TEMP_ALLOCATOR()]].
#define TM_SHUTDOWN_TEMP_ALLOCATOR(ta)                \
    (void)ta##_TM_SHUTDOWN_TEMP_ALLOCATOR_is_missing; \
    tm_temp_allocator_api->destroy(ta);

// Convenience function for allocating memory using [[tm_temp_allocator_i]].
static inline void *tm_temp_alloc(tm_temp_allocator_i *ta, uint64_t sz)
{
    return ta->realloc(ta->inst, 0, 0, sz);
}

// Convenience macro for allocating memory using a frame allocator.
#define tm_frame_alloc(sz) (tm_temp_allocator_api->frame_alloc(sz))

#if defined(TM_LINKS_FOUNDATION)
extern struct tm_temp_allocator_api *tm_temp_allocator_api;
#endif
