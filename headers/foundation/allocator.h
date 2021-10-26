#ifndef FOUNDATION_ALLOCATOR
#define FOUNDATION_ALLOCATOR

#include "api_types.h"

typedef struct tm_allocator_o tm_allocator_o;

// Interface for allocating memory.
typedef struct tm_allocator_i
{
    // Opaque struct storing instance data for this allocator.
    tm_allocator_o *inst;

    // Memory scope used for tracking allocations made with this allocator. The memory scope is
    // initialized by calling [[tm_allocator_api->create_child()]].
    uint32_t mem_scope;
    TM_PAD(4);

    // Basic memory allocator for The Machinery. The [[realloc()]] function provides a functionality
    // similar to stdlib `realloc()`, but not exactly the same.
    //
    // Its arguments are `a`, the [[tm_allocator_i]] structure; `ptr`, a pointer to the block being
    // allocated/reallocated/freed; `old_size`, the current size of the memory block; `new_size`,
    // the new size of the block.
    //
    // Note that the caller of [[realloc()]] is responsible for keeping track of the sizes of memory
    // blocks. This should not be an imposition, because the caller has to know the size of the
    // memory block anyway. Otherwise, it couldn't use it without risking an access violation.
    //
    // The realloc function should implement the following behavior:
    //
    // * When `new_size` is zero, it should behave like `free()` and return `NULL`.
    // * When `new_size` is not zero, it should behave like `realloc()`. In this case it should
    //   return NULL if and only if it can't fulfill the request.
    //
    // The `file` and `line` parameters are used together with the `mem_scope` member for tracking
    // memory allocations. `file` and `line` should typically be set to the C `__FILE__` and
    // `__LINE__` macros.
    //
    // If you are writing a low-level collection class, it is recommended that you don't use
    // `__FILE__` and `__LINE__` directly but instead pass them down from the caller. That way, the
    // memory allocations get attributed to the class using the collection class instead of the
    // collection class itself, which is more useful. See [[carray.inl]] for an example of this.
    //
    // In addition to allocating memory, the allocator is also responsible for registering the
    // allocation with the global memory tracker.
    void *(*realloc)(struct tm_allocator_i *a, void *ptr, uint64_t old_size, uint64_t new_size,
        const char *file, uint32_t line);
} tm_allocator_i;

// Convenience macro for allocating memory using [[tm_allocator_i]].
#define tm_alloc(a, sz) (a)->realloc(a, 0, 0, sz, __FILE__, __LINE__)
#define tm_alloc_at(a, sz, file, line) (a)->realloc(a, 0, 0, sz, file, line)

// Convenience macro for freeing memory using [[tm_allocator_i]].
#define tm_free(a, p, sz) (a)->realloc(a, p, sz, 0, __FILE__, __LINE__)

// Convenience macro for reallocating memory using [[tm_allocator_i]].
#define tm_realloc(a, p, old_sz, new_sz) (a)->realloc(a, p, old_sz, new_sz, __FILE__, __LINE__)

// Allocator statstics.
typedef struct tm_allocator_statistics_t
{
    // Total number of allocations in the `system` allocator.
    TM_ATOMIC uint64_t system_allocation_count;

    // Total allocated bytes in the `system` allocator.
    TM_ATOMIC uint64_t system_allocated_bytes;

    // Total reserved address space in the `vm` and `fixed_vm` allocators.
    TM_ATOMIC uint64_t vm_reserved;

    // Total committed memory in the `vm` and `fixed_vm` allocators.
    TM_ATOMIC uint64_t vm_committed;

    // Allocations in `system` allocator since this counter was externally reset.
    TM_ATOMIC uint64_t system_churn_allocation_count;

    // Bytes allocated in the `system` allocator since this counter was externally reset.
    TM_ATOMIC uint64_t system_churn_allocated_bytes;

    // Virtual memory commited since this counter was externally reset.
    TM_ATOMIC uint64_t vm_churn_committed;
} tm_allocator_statistics_t;

struct tm_allocator_api
{
    // Allocator using the standard system [[realloc()]] function.
    struct tm_allocator_i *system;

    // Allocator used to detect buffer overflow problems. This allocator allocates whole pages from
    // the VM for every allocation and aligns the allocated memory at the end of the page. This
    // means that any code that tries to write beyond the end of the allocated memory triggers a
    // page fault exception. Similarly, writing to freed memory will trigger a page fault -- unless
    // another page has been allocated to the same location which in practice seems rare.
    //
    // If you are having issues with memory overwrites that are hard to pin down, you can try to
    // switch out the regular allocator for the [[end_of_page]] one to see if you can detect the
    // problem.
    struct tm_allocator_i *end_of_page;

    // Allocator that allocates memory directly from the VM. Note that these allocations will always
    // be rounded up to the page size, so it is not a good choice for small allocations. For large
    // allocations it can be a good choice since allocating VM pages will never cause fragmentation.
    // Note though that since allocating from the VM involves a system call and requires the OS to
    // clear the page (for security reasons) it can be slower than other allocators.
    struct tm_allocator_i *vm;

    // Allocator system statistics.
    tm_allocator_statistics_t *statistics;

    // Creates a child allocator of the specified `parent` allocator with the given description.
    // Parent and child allocators form a hierarchy that can be examined in the memory tracker.
    tm_allocator_i (*create_child)(const tm_allocator_i *parent, const char *desc);

    // Destroys a child allocator created by [[create_child()]]. When an allocator is destroyed we
    // verify that all the memory allocated by that allocator has been freed (to prevent memory
    // leaks).
    void (*destroy_child)(const tm_allocator_i *child);

    // As [[destroy_child()]] but allows leaking up to `max_leaked_bytes` bytes without generating an
    // error. This can be used if the allocator is being used by third-party code that is leaking
    // stuff beyond our control.
    //
    // It should not be left permanently in the code, rather the goal should be to get the third
    // party code cleaned up so it doesn't leak anymore.
    void (*destroy_child_allowing_leaks)(const tm_allocator_i *child, uint64_t max_leaked_bytes);

    // Creates a root allocator based on `parent` that is allowed to leak. I.e., [[destroy_child()]]
    // will never be called for the returned allocator and thus we will never check whether it leaks
    // memory or not.
    //
    // This should be used judiciously, because we want to have memory leak detection enabled for as
    // many of our allocators as possible. However it can be useful for some global "singleton"
    // systems that never gets destroyed, such as our list of plugins, etc.
    tm_allocator_i (*create_leaky_root_scope)(const tm_allocator_i *parent, const char *desc);

    // Creates a fixed address VM allocator with the specified maximum size and tracking scope.
    //
    // The fixed address allocator can be used to make allocations that stay at the same address,
    // even if they get realloc:ed to a bigger size. It works by using the virtual memory system to
    // first reserve a large range of address space, and then commit memory out of this reserved
    // space as requested (with calls to `realloc`).
    //
    // The `reserve_size` parameter specifies the amount of address space that is reserved. This is
    // also the maximum amount of memory that an allocation can be realloc:ed to. (Trying to
    // allocate more will result in an error.)
    //
    // The `mem_scope` specifies the tracking scope for the allocator. You can use the scope
    // returned from an existing allocator created by [[create_child()]] or
    // [[create_leaky_root_scope()]].
    //
    // These allocators are useful when you want to allocate an array of objects and ensure that the
    // objects in the array stay at a fixed address in memory, even as the array grows.
    //
    // See also: [[create_leaky_root_scope()]].
    tm_allocator_i (*create_fixed_vm)(uint64_t reserve_size, uint32_t mem_scope);
};

#define tm_allocator_api_version TM_VERSION(1, 0, 0)

#if defined(TM_LINKS_FOUNDATION)
extern struct tm_allocator_api *tm_allocator_api;
#endif


#endif