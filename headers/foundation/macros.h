#pragma once

// Defines some commonly used macros.

// Returns true if the specified macro is defined. Allows you to write ordinary if-statements using
// macros.
#define TM_IS_DEFINED(macro) _TM_IS_DEFINED(macro)

// tm_docgen ignore
#define _TM_IS_DEFINED(macro) (#macro[0] == '1' && #macro[1] == 0)

// Returns the number of items in the static array `a`.
#define TM_ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

// Returns a pointer to the end of the static array `a` (the item past the last element). You can
// use this to loop over the array:
//
// ~~~c
// uint64_t stuff[SIZE];
// for (uint64_t *s = stuff; s != TM_ARRAY_END(stuff); ++s)
//     ...
// ~~~
#define TM_ARRAY_END(a) ((a) + TM_ARRAY_COUNT(a))

#if defined(_MSC_VER)

// Returns the first non-zero item of `(a, b)`.
//
// !!! WARNING
//     `a` might be evaluated more than once by this macro, so don't use an expensive expression
//     or an expression with side effects for `a`. `b` is guaranteed to be evaluated at most once and
//     not evaluated at all if `a` is non-zero, so it's OK to use an expensive expression for `b`.
//
// !!! NOTE
//     It is possible to implement [[tm_or()]] using GCC extensions so that `a` is only evaluated
//     once, but these extensions are not supported by MSVC.
#define tm_or(a, b) ((a) ? (a) : (b))

#else

// tm_docgen ignore
#define tm_or(a, b) ((a) ?: (b))

#endif

// Returns the minimum of `a` and `b`.
#define tm_min(a, b) ((a) < (b) ? (a) : (b))

// Returns the maximum of `a` and `b`.
#define tm_max(a, b) ((a) > (b) ? (a) : (b))

// Clamps `x` to the `[lo, hi]` range.
#define tm_clamp(x, lo, hi) ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : x)

// Returns the median of `(x,a,b)`. [[tm_median()]] is similar to [[tm_clamp()]], but allows arguments
// to be passed in any order at the price of more compare operations.
#define tm_median(x, a, b) tm_clamp((x), tm_min((a), (b)), tm_max((a), (b)))

// True if `lo <= x <= hi`.
#define tm_is_between(x, lo, hi) ((x) >= (lo) && (x) <= (hi))

// Used to mark `a` as an unused variable.
#define tm_unused(a) (void)a

// Returns the offset in bytes of `member` in the struct `structure`.
#define tm_offset_of(structure, member) (uint32_t)((char *)&((structure *)(1024))->member - (char *)1024)

// Returns the size in bytes of `member` in the struct `structure`.
#define tm_sizeof_member(structure, member) ((uint32_t)sizeof(((structure *)(1024))->member))

#if defined(TM_OS_WINDOWS)

#define TM_DIRSEP "\\"

#else

// tm_docgen ignore
#define TM_DIRSEP "/"

#endif