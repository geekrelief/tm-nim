#pragma once

#include "allocator.h"
#include "error.h"
#include "macros.h"
#include "math.inl"

#include <string.h>

// [[hash.inl]] implements typed hash tables and sets in C.
//
// Example:
//
// ~~~c
// struct TM_HASH_T(key_t, value_t) hash = {.allocator = a};
//
// tm_hash_add(&hash, key, val);
// value_t val = tm_hash_get(&hash, key);
// ~~~
//
// The hashes in this file map from an arbitrary 64-bit key type `K` (e.g. `uint64_t`, `T *`,
// [[tm_tt_id_t]], [[tm_entity_t]], ...) to an arbitrary value type `V`.
//
// Only 64-bit key types are supported. If your hash key is smaller, extend it to 64 bits. If your
// hash key is bigger (such as a string), pre-hash it to a 64-bit value and use *that* as your key.
//
// If you use a pre-hash, note that the hash table implementation here doesn't provide any
// protection against collisions in the *pre-hash*. Instead, we just rely on the fact that such
// collisions are statistically improbable.
//
// !!! NOTE
//     If such collisions become a problem in the future, we might add support for 128-bit keys to
//     reduce their probability further.
//
// The hash table uses two sentinel key values to mark unused and deleted keys in the hash table:
// `TM_HASH_UNUSED = 0xffffffffffffffff` and `TM_HASH_TOMBSTONE = 0xfffffffffffffffe`. Note that
// these values can't be used as keys for the hash table. If you are using a hash function to
// generate the key, we again rely on the statistical improbability that it would produce either of
// these values. (You could also modify your hash function so that these values are never produced.)
//
// !!! WARNING: Rvalues
//     Since we take the address of the key passed to [[tm_hash_get()]] you can't use an rvalue for the key:
//
//     ~~~c
//     // Instead of:
//     tm_hash_get(&h, 3);
//
//     // Write:
//     tm_hash_get(&h, (uint64_t){3});
//
//     // Or:
//     tm_hash_get_rv(&h, 3);
//     ~~~
//
// !!! WARNING: Thread-safety
//     For optimization and ease of use, [[tm_hash_get()]] stores a temporary value in the hash structure.
//     This means that it's not thread safe to call [[tm_hash_get()]] on the same hash table from multiple
//     threads simultaneously (even though [[tm_hash_get()]] does not modify the content of the hash
//     table). To be thread-safe, use [[tm_hash_get_ts()]].
//
// !!! NOTE: TODO: Fix this?
//     Should we provide an alternative solution for this? This non-thread safety can be a scary trap.
//
// ### Implementation notes
//
// [[hash.inl]] uses a macro (`TM_TT_HASH_T`) to define a struct with `keys` and `values` arrays as
// well as some additional helper fields.
//
// Macros ([[tm_hash_get()]], etc) are used to provide type safety. By doing things like:
//
// ~~~c
// h->values[i] = value
// ~~~
//
// in the macro, the compiler will ensure that `value` matches the type of `h->values`. Sometimes we
// do this *just* as a type check, i.e. no code will actually execute, with a construct such as:
//
// ~~~c
// 0 && (h->keys[0] = key, 0)
// ~~~
//
// For more advanced operations, the macros call out to C functions. Note that when we do this, all
// the type information is erased, as C doesn't support generics. I.e., all data is passed as `void
// *` with additional information to specify the size (in bytes) of the data.
//
// The keys are required to be unique 64-bit values.
//
// !!! TIP: Use a separate array for large values
//     We allocate the `values` array to the same size as the `keys` array, so if the `keys` array has
//     50 % fillrate, the `values` array will too. This will waste space if the value type is large. If
//     you want to use large values, we suggest you store them in a separate, dense array and just store
//     indices into that array in the hash table.
//
// The `keys` array is always a power-of-two in size. This is important, because it means we can map
// the `key` to an index with a bit shift, which is significantly cheaper than a modulus operation.
// Bit shifts can have bad patterns with certain input keys, such as not using the full key entropy.
// We try to address this with a pre-mix of the key, but it's a bit tricky because doing too much
// pre-mixing will hurt performance. Still trying to find a good balance.
//
// We use linear probing to find they key (or an unused slot in case of an `add`). Removed items are
// marked with a `TOMBSTONE`. Both the `TOMBSTONE` and the `UNUSED` marker are actual keys (with the
// patterns `0xfffffffffffffffe` and `0xffffffffffffffff` respectively). It's illegal to use these
// keys in the hash table.
//
// Currently, the hash table will grow (by doubling in size) when it's 75 % full. This gives an
// average fill rate of (75 + 75/2)/2 = 56 %.
//
// A default value is stored in the struct. This is returned when the key is not found and is by
// default zero-initialized with the rest of the struct.
//
// Rvalue issue
// :    To pass the key to the C functions, we have to take its address with `&key`. Unfortunately, this
//      means that we can't use rvalues for keys. If all our target compiler versions supported the
//      `typeof()` operator, we could fix this in the future by using a temp array for the key:
//
//      ~~~c
//      (typeof(key)[1]){key}
//      ~~~
//
// Thread safety
// :    The hash struct also has a `temp` variable. This is used to store temporary variables inside
//      macros so we don't have to evalute expressions more than once. It's a bit ugly, because it makes
//      the macros that use it not thread-safe (two threads could set different values for `temp`) even
//      if they don't modify any other parts of the hash. Again, this is something that could be fixed
//      with the right compiler support (support for temp variables in macros), which GCC and Clang has,
//      but not MSVC.
//
// To do
// :    Should we support other key widths? 32-bit? 128-bit?
//
// ### Possible improvements
//
// Robin Hood hashing
// :    We could use Robin Hood hashing to shorten searches in the case of
//      collisions. The main benefit of this is to be able to tolerate a higher fill-rate. (With Robin
//      Hood hashing, fill-rates of up to 90 % are possible.) However, since our hash table is always a
//      power of two in size, our average fill rate can theoretically never be higher than 75 % anyway.
//      Adding Robin Hood hashing and growing at 90 % would only up our average fill rate from 56 % to 68
//      %. Doesn't seem like the added complexity is worth that minor improvement.
//
// Add a control word array
// :    We could add a control word array with 1 byte per element
//      containing tombstone/unused flags as well as 7 bits of the hash value, as described in [Designing
//      a Fast, Efficient, Cache-friendly Hash Table](https://www.youtube.com/watch?v=ncHmEUmJZf4&3342).
//      This would bring several advantages:
//
//      * It allows us to to search 16 elements at a time using AVX. (For a match against the stored
//        7 hash bits.)
//
//      * Since we are always searching 16 elements at a time, tombstones can be reused unless the
//        whole 16-element group is full. This makes the hash behave better when there are lots of
//        remove operations.
//
//      * With tombstones and unused elements marked in the control word, we no longer need special
//        key values for them which makes our hashes more universal.
//
//     However, it seems like the control word *mainly* speeds things up when there are collisions.
//     Is the extra overhead worth it when we don't have a lot of collisions? Would need some
//     testing to find out.
//
// Reduce clustering
// :    Linear probing is fast and cache friendly but prone to clustering. Maybe a
//      compromise is to use groups for this too? I.e. search the group (16 elements) linearly and then
//      use a non-linear probe to find the next group to search.

// Sentinel key value that represents a removed key. Note that this value can never be used as an
// actual key, or there will be trouble.
static const uint64_t TM_HASH_TOMBSTONE = 0xfffffffffffffffeULL;

// Sentinel key value that represents an unused key. Note that this value can never be used as an
// actual key, or there will be trouble.
static const uint64_t TM_HASH_UNUSED = 0xffffffffffffffffULL;

// Hash
//
// Macros for manipulating hashes.

// Macro that defines a struct representing a hash from type `K` keys to type `V` values.
//
// You can either use the macro directly to declare a variable or use it to create a typedef.
//
// ~~~c
// struct TM_HASH_T(uint64_t, const char *) my_hash;
//
// typedef struct TM_HASH_T(void *, void *) ptr_lookup;
// ~~~
//
// The macro expands to the following struct:
//
// ~~~c
// struct
// {
//     // Number of items in the `keys` and `values` arrays. Note that this is not the same as
//     // the number of valid items in the hash table, because some items will be `UNUSED`, or
//     // used for `TOMBSTONE`s. You can use `num_buckets` to loop over the `keys` and `values`
//     // arrays to iterate over all hash table items.
//     uint32_t num_buckets;
//
//     // Number of items in the `keys` array used either for real values or `TOMBSTONE`s. (This
//     // is used to determine fill rate.)
//     uint32_t num_used;
//
//     // Temp storage for an index value.
//     uint32_t temp;
//
//     // Arrays of hash table keys and values.
//     //
//     // Note that these arrays are actually allocated together using a single allocation and
//     // thus will be sequential in memory.
//     K *keys;
//     V *values;
//
//     // Allocator used to grow the hash table.
//     //
//     // You can create a statically allocated hash table by statically allocating the `keys`
//     // and `values` arrays and using `NULL` for the `allocator`. A statically allocated hash
//     // table cannot grow beyond the initial `num_buckets` value. Note that you must take care
//     // to use a power-of-two value for `num_buckets` if you set it yourself.
//     struct tm_allocator_i *allocator;
//
//     // Default value of the hash table. This will be returned if you call [[tm_hash_get()]] with a missing
//     // key.
//     V default_value;
// }
// ~~~
#define TM_HASH_T(K, V)                                        \
    {                                                          \
        uint32_t num_buckets;                                  \
        uint32_t num_used;                                     \
        uint32_t temp;                                         \
        uint32_t padding_1;                                    \
        K *keys;                                               \
        V *values;                                             \
        struct tm_allocator_i *allocator;                      \
        V default_value;                                       \
        /* Ensure 8 byte alignment regardless of sizeof(V). */ \
        char padding_2[8 - (sizeof(V) % 8)];                   \
    }

// tm_docgen ignore
//
// Disable warnings produced by our macros.
#if defined(TM_OS_WINDOWS)
#define TM_HASH__DISABLE_WARNINGS() \
    __pragma(warning(push))         \
        __pragma(warning(disable : 4189 4709))
#define TM_HASH__RESTORE_WARNINGS() \
    __pragma(warning(pop))
#else
#define TM_HASH__DISABLE_WARNINGS()
#define TM_HASH__RESTORE_WARNINGS()
#endif

// Returns *true* if `keyptr` should be skipped when iterating over `h` (i.e., `*keyptr` is a `TOMBSTONE`
// or `UNUSED` value):
//
// ~~~c
// for (const K* k = ht.keys; k < ht.keys + ht.num_buckets; ++k) {
//     if (tm_hash_skip_key(&ht, k))
//         continue;
//     ....
// }
// ~~~
#define tm_hash_skip_key(h, keyptr)                       \
    TM_HASH__DISABLE_WARNINGS()                           \
    /* Trick to ensure that `key` has the right type. */  \
    (tm_hash__validate_key_type(h, *(keyptr)),            \
        *(const uint64_t *)(keyptr) >= TM_HASH_TOMBSTONE) \
        TM_HASH__RESTORE_WARNINGS()

// The opposite of [[tm_hash_skip_key()]].
#define tm_hash_use_key(h, keyptr) \
    (!tm_hash_skip_key(h, keyptr))

// Returns *true* if index should be skipped when iterating over the hash table (i.e. if it
// contains a `TOMBSTONE` or `UNUSED` value).
//
// ~~~c
// for (uint32_t i = 0; i < ht.num_buckets; ++i) {
//     if (tm_hash_skip_index(&ht, i))
//         continue;
//     ....
// }
// ~~~
#define tm_hash_skip_index(h, index) \
    tm_hash_skip_key(h, (h)->keys + index)

// The opposite of [[tm_hash_skip_index()]].
#define tm_hash_use_index(h, index) \
    (!tm_hash_skip_index(h, index))

// tm_docgen ignore
//
// Helper macro that doesn't do anything, but checks that key has the right type for the hash.
// A compile error will be generated if key can't be assigned to h->keys[0] or if sizeof(key)
// is not sizeof(h->keys[0])
//
// This must be called within a TM_HASH_DISABLE_WARNINGS() context.
#define tm_hash__validate_key_type(h, key) \
    (0 && ((h)->keys[0] = (key), 0) && sizeof(char[sizeof(key) == sizeof((h)->keys[0]) && sizeof((h)->keys[0]) == 8 ? 1 : -1]))

// Returns the index in the hash table where the `key` is stored, or `UINT32_MAX` if `key` is
// not in the hash table.
#define tm_hash_index(h, key)                                                                     \
    TM_HASH__DISABLE_WARNINGS()                                                                   \
    (tm_hash__validate_key_type(h, key),                                                          \
        tm_hash__index((const uint64_t *)(h)->keys, (h)->num_buckets, *(const uint64_t *)&(key))) \
        TM_HASH__RESTORE_WARNINGS()

// Returns *true* if the hash table contains `key`.
#define tm_hash_has(h, key) (tm_hash_index(h, key) != UINT32_MAX)

// Counts the number of real keys is the hash table.
#define tm_hash_count(h) (tm_hash__num_real_keys((const uint64_t *)(h)->keys, (h)->num_buckets))

// Returns the value stored for `key` in the hash table. If the key has not been stored, the
// `default_value` of the hash table is returned.
//
// !!! WARNING: Not thread-safe
//     This function uses the `temp` field and thus is not safe to call simultaneously from
//     multiple threads. In a multi-thread context, use [[tm_hash_get_ts()]] instead.
#define tm_hash_get(h, key)                           \
    (*(uint32_t *)&(h)->temp = tm_hash_index(h, key), \
        (h)->temp != UINT32_MAX ? (h)->values[(h)->temp] : (h)->default_value)

// A thread-safe version of [[tm_hash_get()]] that uses an explicitly passed temporary variable,
// instead of the one in the `h` structure.
#define tm_hash_get_ts(h, key, temp) \
    (temp = tm_hash_index(h, key),   \
        temp != UINT32_MAX ? (h)->values[temp] : (h)->default_value)

// As [[tm_hash_get()]], but uses the explicitly passed `default_value`, instead of the default value
// set in the hash table.
//
// !!! WARNING: Not thread-safe
//     This function uses the `temp` field and thus is not safe to call simultaneously from
//     multiple threads. In a multi-thread context, use [[tm_hash_get_default_ts()]] instead.
#define tm_hash_get_default(h, key, default_value)    \
    (*(uint32_t *)&(h)->temp = tm_hash_index(h, key), \
        (h)->temp != UINT32_MAX ? (h)->values[(h)->temp] : (default_value))

// A thread-safe version of [[tm_hash_get_default()]] that uses an explicitly passed temporary variable,
// instead of the one in the `h` structure.
#define tm_hash_get_default_ts(h, key, default_value, temp) \
    (temp = tm_hash_index(h, key),                          \
        temp != UINT32_MAX ? (h)->values[temp] : (default_value))

// Adds the pair `(key, value)` to the hash table.
#define tm_hash_add(h, key, value)                                                                                                                                                                \
    TM_HASH__DISABLE_WARNINGS()                                                                                                                                                                   \
    (tm_hash__validate_key_type(h, key),                                                                                                                                                          \
        (h)->temp = tm_hash__add((uint64_t **)&(h)->keys, &(h)->num_used, &(h)->num_buckets, *(uint64_t *)&key, (void **)&(h)->values, sizeof(*(h)->values), (h)->allocator, __FILE__, __LINE__), \
        (h)->values[(h)->temp] = value)                                                                                                                                                           \
        TM_HASH__RESTORE_WARNINGS()

// As [[tm_hash_add()]] but returns a pointer to the value. The returned pointer is valid until the
// hash table is modified. If the key was added to the hash table by this operation, the value is
// set to the default value. If the key already existed, the value remains unchanged.
#define tm_hash_add_reference(h, key)                                                                                                                                                                                                                                              \
    TM_HASH__DISABLE_WARNINGS()                                                                                                                                                                                                                                                    \
    (tm_hash__validate_key_type(h, key),                                                                                                                                                                                                                                           \
        (h)->temp = tm_hash_index(h, key),                                                                                                                                                                                                                                         \
        (h)->temp == UINT32_MAX && ((h)->temp = tm_hash__add((uint64_t **)&(h)->keys, &(h)->num_used, &(h)->num_buckets, *(uint64_t *)&key, (void **)&(h)->values, sizeof(*(h)->values), (h)->allocator, __FILE__, __LINE__), (h)->values[(h)->temp] = (h)->default_value, false), \
        (h)->values + (h)->temp)                                                                                                                                                                                                                                                   \
        TM_HASH__RESTORE_WARNINGS()

// If `key` exists, sets the corresponding `value`, otherwise does nothing. Note that unlike
// [[tm_hash_add()]], [[tm_hash_update()]] will never allocate memory.
#define tm_hash_update(h, key, value)   \
    ((h)->temp = tm_hash_index(h, key), \
        (h)->temp != UINT32_MAX ? (h)->values[(h)->temp] = value : value)

// Removes the value with the specified `key`. Removing a value never reallocates the hash table.
#define tm_hash_remove(h, key) (                                                                                                          \
                                   (h)->temp = tm_hash_index(h, key),                                                                     \
                                   (h)->temp != UINT32_MAX ? ((uint64_t *)(h)->keys)[(h)->temp] = TM_HASH_TOMBSTONE : TM_HASH_TOMBSTONE), \
                               (h)->num_used -= (h)->temp != UINT32_MAX ? 1 : 0

// Clears the hash table. This does not free any memory. The array will keep the same number of
// buckets, but all the keys and values will be erased.
#define tm_hash_clear(h) (memset((h)->keys, 0xff, (h)->num_buckets * sizeof(*(h)->keys)), (h)->num_used = 0)

// Frees the hash table.
#define tm_hash_free(h) (tm_hash__free((uint64_t **)&(h)->keys, &(h)->num_used, &(h)->num_buckets, (void **)&(h)->values, sizeof(*(h)->values), (h)->allocator, __FILE__, __LINE__))

// Copies the hash table `from` to `h`.
#define tm_hash_copy(h, from) \
    (tm_hash_free(h),         \
        *h = *from,           \
        tm_hash__copy((uint64_t **)&(h)->keys, (h)->num_buckets, (void **)&(h)->values, sizeof(*(h)->values), (h)->allocator, __FILE__, __LINE__))

// Compute the number of buckets required to accommodate `n` elements.
//
// tm_docgen ignore
static inline uint32_t private__buckets_for_elements(uint32_t n)
{
    return tm_uint32_round_up_to_power_of_two(tm_max(16, (uint32_t)(n / 0.7f + 1)));
}

// Reserves space in the hash table for `num_elements` new elements. The hash table will be resized
// so that its size is a power-of-two and adding `num_elements` elements will keep it below the
// target fillrate (currently 70 %). If the hash table already has room for `num_elements` new
// elements, this function is a NOP.
#define tm_hash_reserve(h, num_elements) \
    tm_hash__grow_to((uint64_t **)&(h)->keys, &(h)->num_used, &(h)->num_buckets, (void **)&(h)->values, sizeof(*(h)->values), (h)->allocator, __FILE__, __LINE__, private__buckets_for_elements((h)->num_used + num_elements))

// Set
//
// Macros for set manipulation.

// Macro that defines a struct representing a set of `K` keys.
//
// The set implementation is identical to the hash table implementation, except it doesn't have a
// `values` array.
#define TM_SET_T(K)                       \
    {                                     \
        uint32_t num_buckets;             \
        uint32_t num_used;                \
        uint32_t temp;                    \
        uint32_t padding;                 \
        K *keys;                          \
        struct tm_allocator_i *allocator; \
    }

// Returns the index in the set where the `key` is stored, or `UINT32_MAX` if `key` is not in the
// set.
#define tm_set_index(h, key) tm_set_index(h, key)

// Returns *true* if the set contains `key`.
#define tm_set_has(h, key) tm_hash_has(h, key)

// Counts the number of real keys is the set.
#define tm_set_count(h) tm_hash_count(h)

// Adds the `key` to the set. Note that this might reallocate the set.
#define tm_set_add(h, key) (tm_hash__add((uint64_t **)&(h)->keys, &(h)->num_used, &(h)->num_buckets, *(uint64_t *)&key, NULL, 0, (h)->allocator, __FILE__, __LINE__)[(h)->keys] = key)

// Removes the value with the specified `key`. Removing a value never reallocates set.
#define tm_set_remove(h, key) tm_hash_remove(h, key)

// Clears the set. This does not free any memory.
#define tm_set_clear(h) tm_hash_clear(h)

// Frees the set.
#define tm_set_free(h) (tm_hash__free((uint64_t **)&(h)->keys, &(h)->num_used, &(h)->num_buckets, NULL, 0, (h)->allocator, __FILE__, __LINE__))

// Copies the set `from` to `h`.
#define tm_set_copy(h, from) \
    (tm_set_free(h),         \
        *h = *from,          \
        tm_hash__copy((uint64_t **)&(h)->keys, (h)->num_buckets, 0, 0, (h)->allocator, __FILE__, __LINE__))

// Reserves space in the set for `num_elements` new elements.
#define tm_set_reserve(h, num_elements) \
    tm_hash__grow_to((uint64_t **)&(h)->keys, &(h)->num_used, &(h)->num_buckets, 0, 0, (h)->allocator, __FILE__, __LINE__, private__buckets_for_elements((h)->num_used + num_elements))

// Identical to [[tm_hash_skip_index()]].
#define tm_set_skip_index(s, index) tm_hash_skip_index(s, index)

// Identical to [[tm_hash_use_index()]].
#define tm_set_use_index(s, index) tm_hash_use_index(s, index)

// Standard hash and set types
//
// Defines some commonly used set and hash types.

// Maps from an `uint64_t` key to an `uint64_t` value.
typedef struct tm_hash64_t TM_HASH_T(uint64_t, uint64_t) tm_hash64_t;

// Maps from an `uint64_t` key to a `uint32_t` value.
typedef struct tm_hash32_t TM_HASH_T(uint64_t, uint32_t) tm_hash32_t;

// Maps from an `uint64_t` key to a `float` value.
typedef struct tm_hash64_float_t TM_HASH_T(uint64_t, float) tm_hash64_float_t;

// Maps from an [[tm_tt_id_t]] key to a [[tm_tt_id_t]] value.
typedef struct tm_hash_id_to_id_t TM_HASH_T(tm_tt_id_t, tm_tt_id_t) tm_hash_id_to_id_t;

// Represents a set of `uint64_t` keys.
typedef struct tm_set_t TM_SET_T(uint64_t) tm_set_t;

// Represents a set of [[tm_tt_id_t]] keys.
typedef struct tm_set_id_t TM_SET_T(tm_tt_id_t) tm_set_id_t;

// Represents a set of hashed strings.
typedef struct tm_set_strhash_t TM_SET_T(tm_strhash_t) tm_set_strhash_t;

// Rvalue macros
//
// These macros can be used when you have an `uint64_t` rvalue that you want to use as the key. They
// automatically wrap the key in `(uint64_t){ key }`. Note that if the key type of your hash is
// something other than an `uint64_t`, these macros won't work.

#define tm_hash_has_rv(h, key) tm_hash_has(h, (uint64_t){ key })

#define tm_hash_get_rv(h, key) tm_hash_get(h, (uint64_t){ key })

#define tm_hash_get_default_rv(h, key, default_value) tm_hash_get_default(h, (uint64_t){ key }, default_value)

#define tm_hash_get_rv_ts(h, key, temp) tm_hash_get_ts(h, (uint64_t){ key }, temp)

#define tm_hash_add_rv(h, key, value) tm_hash_add(h, (uint64_t){ key }, value)

#define tm_hash_add_reference_rv(h, key) tm_hash_add_reference(h, (uint64_t){ key })

#define tm_hash_update_rv(h, key, value) tm_hash_update(h, (uint64_t){ key }, value)

#define tm_hash_remove_rv(h, key) tm_hash_remove(h, (uint64_t){ key })

#define tm_set_has_rv(h, key) tm_set_has(h, (uint64_t){ key })

#define tm_set_add_rv(h, key) tm_set_add(h, (uint64_t){ key })

#define tm_set_remove_rv(h, key) tm_set_remove(h, (uint64_t){ key })

// tm_docgen off

// Function implementations.

// TODO: This is a bit ugly -- assumes that tm_error_api is always available as an extern struct
//       in any file that wants to use [[hash.inl]]. But hard to come up with a good alternative...
extern struct tm_error_api *tm_error_api;

// Maps a key to a hash table index.
//
// ???: Should we do more to mix up the bits of `key` here? We want to make sure that we don't have
// any bad patterns in the bits, but this function gets called a lot, so we don't want to add too
// many steps here or performance will suffer.
static inline uint32_t tm_hash__first_index(uint64_t key, uint32_t num_buckets)
{
    uint32_t *v = (uint32_t *)&key;
    return (v[0] ^ v[1]) & (num_buckets - 1);
}

// Frees the arrays allocated by the hash.
static inline void tm_hash__free(uint64_t **keys_ptr, uint32_t *num_used_ptr, uint32_t *num_buckets_ptr, void **values_ptr, uint64_t value_bytes, tm_allocator_i *allocator, const char *file, uint32_t line)
{
    uint64_t *keys = *keys_ptr;
    const uint32_t num_buckets = *num_buckets_ptr;
    const uint64_t bytes_to_free = num_buckets ? num_buckets * (sizeof(*keys) + value_bytes) : 0;
    if (allocator)
        allocator->realloc(allocator, keys, bytes_to_free, 0, file, line);
    *keys_ptr = 0;
    if (values_ptr)
        *values_ptr = 0;
    *num_used_ptr = 0;
    *num_buckets_ptr = 0;
}

// Returns the index of the `key` in the array `keys` or `UINT32_MAX` if `key` is not in the array.
static inline uint32_t tm_hash__index(const uint64_t *keys, uint32_t num_buckets, uint64_t key)
{
    if (!num_buckets || key >= TM_HASH_TOMBSTONE)
        return UINT32_MAX;

    const uint32_t max_distance = num_buckets;

    uint32_t i = tm_hash__first_index(key, num_buckets);
    uint32_t distance = 0;
    while (keys[i] != key) {
        if (distance > max_distance)
            return UINT32_MAX;
        if (keys[i] == TM_HASH_UNUSED)
            return UINT32_MAX;
        i = (i + 1) & (num_buckets - 1);
        ++distance;
    }
    return i;
}

// Tries to add `key` to the `keys` array (without reallocating it). If it succeeds, it will return
// the index of the added key and otherwise `UINT32_MAX`. You should only call this if
// [[tm_hash_index()]] has failed to find the key.
static inline uint32_t tm_hash__add_no_grow(const uint64_t *keys, uint32_t num_buckets, uint64_t key)
{
    const uint32_t max_distance = num_buckets;

    if (!num_buckets)
        return UINT32_MAX;

    uint32_t i = tm_hash__first_index(key, num_buckets);
    uint32_t distance = 0;
    while (keys[i] != TM_HASH_UNUSED && keys[i] != TM_HASH_TOMBSTONE) {
        if (distance > max_distance)
            return UINT32_MAX;
        i = (i + 1) & (num_buckets - 1);
        ++distance;
    }
    return i;
}

static inline void tm_hash__grow(uint64_t **keys_ptr, uint32_t *num_used_ptr, uint32_t *num_buckets_ptr, void **values_ptr, uint64_t value_bytes, tm_allocator_i *allocator, const char *file, uint32_t line);

// Adds `key` to the hash table, growing it as necessary, and returns its index. If the hash table
// needs to grow and `allocator` is NULL, an error will be generated.
static inline uint32_t tm_hash__add(uint64_t **keys_ptr, uint32_t *num_used_ptr, uint32_t *num_buckets_ptr, uint64_t key, void **values_ptr, uint64_t value_bytes, tm_allocator_i *allocator, const char *file, uint32_t line)
{
    uint32_t i = tm_hash__index(*keys_ptr, *num_buckets_ptr, key);
    if (i != UINT32_MAX)
        return i;

    if ((!*num_buckets_ptr || (float)*num_used_ptr / (float)*num_buckets_ptr > 0.7f) && allocator)
        tm_hash__grow(keys_ptr, num_used_ptr, num_buckets_ptr, values_ptr, value_bytes, allocator, file, line);

    if (!TM_ASSERT(key != TM_HASH_UNUSED && key != TM_HASH_TOMBSTONE, tm_error_api->def, "Tombstone cannot be used as key"))
        return 0;

    i = tm_hash__add_no_grow(*keys_ptr, *num_buckets_ptr, key);
    if (!TM_ASSERT(i != UINT32_MAX, tm_error_api->def, "%s", values_ptr ? "Static hash full" : "Static set full"))
        return 0;

    uint64_t *keys = *keys_ptr;
    keys[i] = key;
    ++*num_used_ptr;
    return i;
}

// Grows the hash table to the specified size
static inline void tm_hash__grow_to(uint64_t **keys_ptr, uint32_t *num_used_ptr, uint32_t *num_buckets_ptr, void **values_ptr, uint64_t value_bytes, tm_allocator_i *allocator, const char *file, uint32_t line, uint32_t new_buckets)
{
    const uint64_t *keys = *keys_ptr;
    const uint32_t num_buckets = *num_buckets_ptr;
    const void *values = values_ptr ? *values_ptr : 0;

    if (num_buckets >= new_buckets)
        return;

    const uint64_t to_alloc = new_buckets * (sizeof(*keys) + value_bytes);
    uint64_t *new_keys = (uint64_t *)allocator->realloc(allocator, 0, 0, to_alloc, file, line);
    void *new_values = new_keys + new_buckets;
    memset(new_keys, 0xff, new_buckets * sizeof(uint64_t));
    uint32_t new_elements = 0;

    for (uint32_t i = 0; i < num_buckets; ++i) {
        if (keys[i] != TM_HASH_UNUSED && keys[i] != TM_HASH_TOMBSTONE) {
            const uint32_t new_i = tm_hash__add_no_grow(new_keys, new_buckets, keys[i]);
            new_keys[new_i] = keys[i];
            if (value_bytes)
                memcpy((char *)new_values + new_i * value_bytes, (char *)values + i * value_bytes, value_bytes);
            ++new_elements;
        }
    }
    tm_hash__free(keys_ptr, num_used_ptr, num_buckets_ptr, values_ptr, value_bytes, allocator, file, line);
    *num_used_ptr = new_elements;
    *num_buckets_ptr = new_buckets;
    *keys_ptr = new_keys;
    if (values_ptr)
        *values_ptr = new_values;
}

// Grows the hash table.
static inline void tm_hash__grow(uint64_t **keys_ptr, uint32_t *num_used_ptr, uint32_t *num_buckets_ptr, void **values_ptr, uint64_t value_bytes, tm_allocator_i *allocator, const char *file, uint32_t line)
{
    const uint32_t num_used = *num_used_ptr;
    const uint32_t num_buckets = *num_buckets_ptr;

    uint32_t new_buckets = num_buckets ? num_buckets : 16;
    while ((float)num_used / new_buckets > 0.5f)
        new_buckets *= 2;

    tm_hash__grow_to(keys_ptr, num_used_ptr, num_buckets_ptr, values_ptr, value_bytes, allocator, file, line, new_buckets);
}

// Copies the keys and values arrays to new pointers.
static inline void tm_hash__copy(uint64_t **keys_ptr, uint32_t num_buckets, void **values_ptr,
    uint64_t value_bytes, tm_allocator_i *allocator, const char *file, uint32_t line)
{
    uint64_t *old_keys = *keys_ptr;
    const uint64_t size = num_buckets * (sizeof(old_keys) + value_bytes);
    *keys_ptr = (uint64_t *)allocator->realloc(allocator, 0, 0, size, file, line);

    if (values_ptr)
        *values_ptr = (*keys_ptr) + num_buckets;

    memcpy(*keys_ptr, old_keys, size);
}

static inline uint32_t tm_hash__num_real_keys(const uint64_t *keys, uint32_t num_buckets)
{
    uint32_t num_keys = 0;
    for (uint32_t i = 0; i < num_buckets; ++i)
        num_keys += (keys[i] < TM_HASH_TOMBSTONE);
    return num_keys;
}