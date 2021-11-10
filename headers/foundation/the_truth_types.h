#ifndef FOUNDATION_THE_TRUTH_TYPES
#define FOUNDATION_THE_TRUTH_TYPES

#include "api_types.h"

// Defines some commonly used types for The Truth.
//
// !!! TODO: API-REVIEW
//     Rename this file to `the_truth_common_types.h`.

// Scalar types
//
// These types only have a single property at index 0 of the specified type.

#define TM_TT_TYPE__BOOL "tm_bool"
#define TM_TT_TYPE_HASH__BOOL TM_STATIC_HASH("tm_bool", 0xaed3caa5c516d191ULL)

#define TM_TT_TYPE__UINT32_T "tm_uint32_t"
#define TM_TT_TYPE_HASH__UINT32_T TM_STATIC_HASH("tm_uint32_t", 0xeaf325d06c5cb568ULL)

#define TM_TT_TYPE__UINT64_T "tm_uint64_t"
#define TM_TT_TYPE_HASH__UINT64_T TM_STATIC_HASH("tm_uint64_t", 0xb11aa2e510ce635eULL)

#define TM_TT_TYPE__FLOAT "tm_float"
#define TM_TT_TYPE_HASH__FLOAT TM_STATIC_HASH("tm_float", 0x6d0a4e744c45523bULL)

#define TM_TT_TYPE__DOUBLE "tm_double"
#define TM_TT_TYPE_HASH__DOUBLE TM_STATIC_HASH("tm_double", 0xef2dd9a55accbe4ULL)

#define TM_TT_TYPE__STRING "tm_string"
#define TM_TT_TYPE_HASH__STRING TM_STATIC_HASH("tm_string", 0xa84ae1fca1a3e0cbULL)

// Vector types

#define TM_TT_TYPE__VEC2 "tm_vec2_t"
#define TM_TT_TYPE_HASH__VEC2 TM_STATIC_HASH("tm_vec2_t", 0x5ea1bb7b6537de46ULL)

enum {
    TM_TT_PROP__VEC2__X, // float
    TM_TT_PROP__VEC2__Y, // float
};

#define TM_TT_TYPE__VEC3 "tm_vec3_t"
#define TM_TT_TYPE_HASH__VEC3 TM_STATIC_HASH("tm_vec3_t", 0x8d1487af36b1e3e1ULL)

enum {
    TM_TT_PROP__VEC3__X, // float
    TM_TT_PROP__VEC3__Y, // float
    TM_TT_PROP__VEC3__Z, // float
};

#define TM_TT_TYPE__VEC4 "tm_vec4_t"
#define TM_TT_TYPE_HASH__VEC4 TM_STATIC_HASH("tm_vec4_t", 0xdf81286b1233bab6ULL)

enum {
    TM_TT_PROP__VEC4__X, // float
    TM_TT_PROP__VEC4__Y, // float
    TM_TT_PROP__VEC4__Z, // float
    TM_TT_PROP__VEC4__W, // float
};

#define TM_TT_TYPE__POSITION "tm_position"
#define TM_TT_TYPE_HASH__POSITION TM_STATIC_HASH("tm_position", 0x7a29b8f6b1ca42ecULL)

enum {
    TM_TT_PROP__POSITION__X, // float
    TM_TT_PROP__POSITION__Y, // float
    TM_TT_PROP__POSITION__Z, // float
};

#define TM_TT_TYPE__ROTATION "tm_rotation"
#define TM_TT_TYPE_HASH__ROTATION TM_STATIC_HASH("tm_rotation", 0xa4d2f46b41c9d717ULL)

enum {
    TM_TT_PROP__ROTATION__X, // float
    TM_TT_PROP__ROTATION__Y, // float
    TM_TT_PROP__ROTATION__Z, // float
    TM_TT_PROP__ROTATION__W, // float
};

#define TM_TT_TYPE__SCALE "tm_scale"
#define TM_TT_TYPE_HASH__SCALE TM_STATIC_HASH("tm_scale", 0x20ef513a9606cc30ULL)

enum {
    TM_TT_PROP__SCALE__X, // float
    TM_TT_PROP__SCALE__Y, // float
    TM_TT_PROP__SCALE__Z, // float
};

#define TM_TT_TYPE__COLOR_RGB "tm_color_rgb"
#define TM_TT_TYPE_HASH__COLOR_RGB TM_STATIC_HASH("tm_color_rgb", 0xd0c8704d11c0c0f1ULL)

enum {
    TM_TT_PROP__COLOR_RGB__R, // float
    TM_TT_PROP__COLOR_RGB__G, // float
    TM_TT_PROP__COLOR_RGB__B, // float
};

#define TM_TT_TYPE__COLOR_RGBA "tm_color_rgba"
#define TM_TT_TYPE_HASH__COLOR_RGBA TM_STATIC_HASH("tm_color_rgba", 0x11e53fa0440a07e4ULL)

enum {
    TM_TT_PROP__COLOR_RGBA__R, // float
    TM_TT_PROP__COLOR_RGBA__G, // float
    TM_TT_PROP__COLOR_RGBA__B, // float
    TM_TT_PROP__COLOR_RGBA__A, // float
};

#define TM_TT_TYPE__RECT "tm_rect_t"
#define TM_TT_TYPE_HASH__RECT TM_STATIC_HASH("tm_rect_t", 0xa6e9c5f4ec093df1ULL)

enum {
    TM_TT_PROP__RECT__X, // float
    TM_TT_PROP__RECT__Y, // float
    TM_TT_PROP__RECT__W, // float
    TM_TT_PROP__RECT__H, // float
};

#define TM_TT_TYPE__UUID "tm_uuid_t"
#define TM_TT_TYPE_HASH__UUID TM_STATIC_HASH("tm_uuid_t", 0x63d44d07a8f8ea75ULL)

enum {
    TM_TT_PROP__UUID_A, // uint64_t
    TM_TT_PROP__UUID_B // uint64_t
};

struct tm_the_truth_o;
struct tm_the_truth_object_o;

struct tm_the_truth_common_types_api
{
    // Creates the common types in the specified truth object.
    void (*create_common_types)(struct tm_the_truth_o *tt);

    // Property getters
    //
    // These functions read a subobject in the `property` of the `object` as one of the vector
    // types defined in this file.

    tm_vec2_t (*get_vec2)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_vec3_t (*get_vec3)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_vec4_t (*get_vec4)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_rect_t (*get_rect)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_vec3_t (*get_position)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_vec4_t (*get_rotation)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_vec3_t (*get_scale)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_vec3_t (*get_color_rgb)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_vec4_t (*get_color_rgba)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_color_srgb_t (*get_color_srgb)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_color_srgb_t (*get_color_srgba)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    tm_uuid_t (*get_uuid)(const struct tm_the_truth_o *tt,
        const struct tm_the_truth_object_o *object, uint32_t property);

    // Property setters
    //
    // These functions write a subobject in the `property` of the `object` as one of the vector
    // types in this file. If the subobject is currently a NULL object, a new object is created. The
    // write is logged under the specified `undo_scope`. Use [[TM_TT_NO_UNDO_SCOPE]] if you don't need
    // the action to be undoable.

    void (*set_vec2)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_vec2_t vec2, tm_tt_undo_scope_t undo_scope);

    void (*set_vec3)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_vec3_t vec3, tm_tt_undo_scope_t undo_scope);

    void (*set_vec4)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_vec4_t vec4, tm_tt_undo_scope_t undo_scope);

    void (*set_rect)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_rect_t rect, tm_tt_undo_scope_t undo_scope);

    void (*set_position)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_vec3_t vec3, tm_tt_undo_scope_t undo_scope);

    void (*set_rotation)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_vec4_t vec4, tm_tt_undo_scope_t undo_scope);

    void (*set_scale)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_vec3_t vec3, tm_tt_undo_scope_t undo_scope);

    void (*set_color_rgb)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_vec3_t vec3, tm_tt_undo_scope_t undo_scope);

    void (*set_color_rgba)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_vec4_t vec4, tm_tt_undo_scope_t undo_scope);

    void (*set_color_srgb)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_color_srgb_t col, tm_tt_undo_scope_t undo_scope);

    void (*set_color_srgba)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_color_srgb_t col, tm_tt_undo_scope_t undo_scope);

    void (*set_uuid)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *object,
        uint32_t property, tm_uuid_t uuid, tm_tt_undo_scope_t undo_scope);

    // Helper functions
    //
    // These functions can help with reading/writing Truth objects that only have float properties.

    // Reads the first `n` properties of `obj` as floats into `res`. `res` should have room for `n`
    // floats. Returns a pointer to `res`.
    float *(*read_floats)(const struct tm_the_truth_o *tt, const struct tm_the_truth_object_o *obj,
        float *res, uint32_t n);

    // Writes the first `n` properties of `obj` as floats from the `values` array.
    void (*write_floats)(struct tm_the_truth_o *tt, struct tm_the_truth_object_o *obj,
        const float *values, uint32_t n);
};

#define tm_the_truth_common_types_api_version TM_VERSION(1, 0, 0)

#if defined(TM_LINKS_FOUNDATION)
extern struct tm_the_truth_common_types_api *tm_the_truth_common_types_api;
#endif


#endif