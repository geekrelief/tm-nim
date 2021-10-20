#pragma once

#include "api_types.h"

struct tm_allocator_i;
struct tm_os_file_io_api;
struct tm_file_o;

// API for dealing with "buffers". Buffers are big binary blobs of memory (textures, vertex
// data, etc). The Buffer API provides reference counting and streaming for buffers.

typedef struct tm_buffers_o tm_buffers_o;

// Manages a set of "buffers". A buffer is an immutable blob of binary data .
//
// The buffer API is thread-safe, but it is up to the caller of the API to make sure that the *use*
// of the buffers is thread-safe. (I.e., that no-one is currently reading the content of a buffer
// that is being released.)
//
// !!! TODO: API-REVIEW
//     Should we get rid of the concept of a buffer ID and just refer to buffers by their hash?
typedef struct tm_buffers_i
{
    tm_buffers_o *inst;

    // Allocates a buffer of `size` bytes to be passed into the [[add()]] function. If `initialize` is
    // non-zero, the buffer will be initialized with data copied from the `initialize` pointer.
    // Otherwise you are responsible for filling the buffer with data yourself before calling
    // [[add()]].
    //
    // Note that you must call [[add()]] with the allocated data. If you don't, the allocated memory
    // is leaked.
    void *(*allocate)(tm_buffers_o *inst, uint64_t size, const void *initialize);

    // Adds a buffer containing the specified `data` of size `size`. Returns an ID identifying the
    // new buffer. If you know the hash of the data, you can pass it in the `hash` paramter.
    // Otherwise, if you pass `0` for hash, the hash of the data will be computed automatically.
    //
    // The data passed to [[add()]] must come from an [[allocate()]] call. [[add()]] will take over
    // ownership of the data. You cannot reuse the data pointer for two different [[add()]] calls.
    uint32_t (*add)(tm_buffers_o *inst, const void *data, uint64_t size, uint64_t hash);

    // Increases the buffer's reference count by 1. (The buffer starts at 1 after [[add()]].)
    void (*retain)(tm_buffers_o *inst, uint32_t id);

    // Decreases the buffer's reference count by 1. When the reference count reaches zero, the
    // buffer is destroyed.
    void (*release)(tm_buffers_o *inst, uint32_t id);

    // Returns the content of the buffer. The size is returned in `size`.
    const void *(*get)(const tm_buffers_o *inst, uint32_t id, uint64_t *size);

    // Returns the size of the buffer.
    uint64_t (*size)(const tm_buffers_o *inst, uint32_t id);

    // Returns the hash of the buffer.
    uint64_t (*hash)(const tm_buffers_o *inst, uint32_t id);

    // If a buffer exists with the specified hash, returns its ID. Otherwise, returns zero.
    uint32_t (*lookup)(const tm_buffers_o *inst, uint64_t hash);

    // For debugging purposes only. Returns the reference count of the buffer.
    uint32_t (*debug__refcount)(const tm_buffers_o *inst, uint32_t id);
} tm_buffers_i;

// Manages a set of streamable buffers. Streamable buffers work as regular buffers except that they
// can optionally be created with disk backing. Buffers with disk backing can be loaded on-demand or
// streamed in by a background process. They can also be flushed out of main memory as necessary
// (since they are backed up on disk).
//
// * Calling [[get()]] or [[hash()]] on a buffer that isn't loaded will on-demand load the buffer before
//   the function returns. Note that this can stall the application, so you might want to pre-load
//   buffers by calling [[get()]] on them in a background thread before they're actually needed.
typedef struct tm_streamable_buffers_i
{
    // Streamable buffers support the regular buffers interface.
    TM_INHERITS(struct tm_buffers_i);

    // Creates a streamable buffer with the content mapped to the specified region (`offset`,
    // `size`) of the specified `path`. The buffer is created in "unloaded" state (not mapped to
    // main memory). It will be loaded on demand by a subsequent [[get()]] operation. If `size` is
    // zero, the size of the buffer will automatically be set from the size of the mapped file.
    //
    // If `hash` is non-zero, the specified value is used for the hash of the data, otherwise, the
    // hash is computed for the data.
    uint32_t (*map)(tm_buffers_o *inst, const char *path, uint64_t offset, uint64_t size,
        uint64_t hash);

    // Creates a streamable buffer mapped to an asset database (see [[asset_database.h]]).
    //
    // `hash` and `size` specify the hash and size of the buffer. The buffer is stored in a sequence
    // of linked "pages" of size `page_size`. Pages have `page_header_size` byte header where the
    // first four bytes are the index of the next page in the sequence. After the header bytes, the
    // data for the page follows.
    uint32_t (*map_database)(tm_buffers_o *inst, uint64_t hash, uint64_t size, const struct tm_file_o *file,
        uint32_t page_size, uint32_t page_header_size, uint32_t first_page);

    // Returns *true* if the specified buffer is backed by file data, *false* if it only exists in
    // memory.
    bool (*is_mapped)(const tm_buffers_o *inst, uint32_t id);

    // Returns *true* if the specified buffer has been loaded into main memory, *false* if it only
    // exists on disk.
    bool (*is_loaded)(const tm_buffers_o *inst, uint32_t id);

    // If the `buffer` is currently loaded into main memory -- unloads it from main memory. You can
    // only call this on buffers that are mapped to disk, because we need to be able to get the
    // content back on-demand.
    //
    // !!! TODO: API-REVIEW
    //     We might need a concept of "buffers in use" to know when its safe to unload buffers.
    void (*unload)(tm_buffers_o *inst, uint32_t id);

    // Saves the content of the buffer to the specified file and offset. This creates a mapping
    // between the buffer and the file.
    void (*save)(tm_buffers_o *inst, uint32_t id, const char *path, uint64_t offset);

    // Preloads all buffers on a background thread.
    // If `percentage` is not null, it will be filled continously with the current buffer loading percentage.
    void (*background_load_all)(tm_buffers_o *inst, float *percentage);

    // Waits until all buffer data has been loaded.
    void (*ensure_all_loaded)(tm_buffers_o *inst);

    // Sets the IO interface to use for file access This is mostly used for testing with mock file
    // systems. If `io` is `NULL`, the default file system interface ([[tm_os_api->file_system]]) will
    // be used.
    void (*set_io)(tm_buffers_o *inst, struct tm_os_file_io_api *io);
} tm_streamable_buffers_i;

// API for creating [[tm_buffers_i]] and [[tm_streamable_buffers_i]] objects.
struct tm_buffers_api
{
    // Creates a new buffer container.
    struct tm_buffers_i *(*create)(struct tm_allocator_i *a);

    // Destroys a buffer container created by [[create()]].
    void (*destroy)(struct tm_buffers_i *i);

    // Creates a new streamable buffer container. If `io` is specified, it will be used for all file
    // operations. Otherwise, [[tm_os_api->file_io]] will be used.
    struct tm_streamable_buffers_i *(*create_streamable)(struct tm_allocator_i *a, struct tm_os_file_io_api *io);

    // Destroys a buffer container created by [[create_streamable()]].
    void (*destroy_streamable)(struct tm_streamable_buffers_i *i);
};

#define tm_buffers_api_version TM_VERSION(1, 0, 0)

#if defined(TM_LINKS_FOUNDATION)
extern struct tm_buffers_api *tm_buffers_api;
#endif
