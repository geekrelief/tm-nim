#ifndef FOUNDATION_MATH
#define FOUNDATION_MATH

TM_DISABLE_PADDING_WARNINGS

#if defined(TM_CPU_SSE) || defined(TM_CPU_AVX)
#include <immintrin.h>
#elif defined(TM_CPU_NEON)
// Use sse2neon to emulate SSE on NEON.
//
// Investigate: is this a good idea? This talk from Google Filament says that it's better to NOT use
// Neon intrinsics at all and instead rely on compiler auto-vectorization by using --ffast-math,
// --ffp-contract=fast and __restrict__. But I'm not sure if that's the case:
//
// https://www.youtube.com/watch?v=Lcq_fzet9Iw&t=130s
#include "sse2neon.h"
#define TM_CPU_SSE 1
#endif

#if defined(TM_OS_WINDOWS)
#include <intrin.h>
#endif

TM_RESTORE_PADDING_WARNINGS

#include <math.h>
#include <string.h>

// Math library for basic operations on vectors, matrices and quaternions.

// Constants
//
// Mathematical constants.

// Single precision pi.
#define TM_PI 3.14159265f

// Double precision pi.
#define TM_PI_D 3.14159265358979323846

// Vec2

// Computes the piecewise sum of the two specified 2D vectors.
// $ \mathbf{A}+\mathbf{B}=(a_x+b_x,a_y+b_y) $
static inline tm_vec2_t tm_vec2_add(tm_vec2_t lhs, tm_vec2_t rhs);

// Computes the difference of the two specified 2D vectors.
// $ \mathbf{A}-\mathbf{B}=(a_x-b_x,a_y-b_y) $
static inline tm_vec2_t tm_vec2_sub(tm_vec2_t lhs, tm_vec2_t rhs);

// Computes the dot product of the two specified 2D vectors.
// $ \mathbf{A}\cdot\mathbf{B}=(a_x\times b_x+a_y\times b_y) $
static inline float tm_vec2_dot(tm_vec2_t v1, tm_vec2_t v2);

// Computes the perp-dot product of the two specified 2D vectors.
// This is defined as the dot product of the perpendicular vector of `v1` and `v2`.
// Often this function is referred to as the 2D version of the cross product.
// $ \mathbf{A}^{\perp}\cdot\mathbf{B}=(a_x\times b_y-a_y\times b_x) $
static inline float tm_vec2_cross(tm_vec2_t v1, tm_vec2_t v2);

// Multiples the elements of vector `lhs` by scalar `rhs`.
// $ \mathbf{A}\times t=(a_x\times t, a_y\times t) $
static inline tm_vec2_t tm_vec2_mul(tm_vec2_t lhs, float rhs);

// Performs a multiply and add operation on the specified values.
// $ \mathbf{A}+t\mathbf{B}=(a_x+t\times b_x,a_y+t\times b_y) $
static inline tm_vec2_t tm_vec2_mul_add(tm_vec2_t lhs, tm_vec2_t rhs, float mul);

// Performs a piecewise multiplication of the two specified 2D vectors.
// $ \mathbf{A}\times\mathbf{B}=(a_x\times b_x, a_y\times b_y) $
static inline tm_vec2_t tm_vec2_element_mul(tm_vec2_t lhs, tm_vec2_t rhs);

// Performs a piecewise division of the two specified 2D vectors.
// $ \frac{\mathbf{A}}{\mathbf{B}}=(\frac{a_x}{b_x},\frac{a_y}{b_y}) $
static inline tm_vec2_t tm_vec2_element_div(tm_vec2_t lhs, tm_vec2_t rhs);

// Computes the magnitude (length) of the specified 2D vector `v`.
// $ \left|\mathbf{V}\right|=\sqrt{v_x^2+v_y^2} $
static inline float tm_vec2_length(tm_vec2_t v);

// Computes the normalized 2D vector of `v`. Returns zero for very small vectors.
// $ \hat{\mathbf{V}}=\frac{\mathbf{V}}{\left|\mathbf{V}\right|} $
static inline tm_vec2_t tm_vec2_normalize(tm_vec2_t v);

// Checks the binary equivalence of the two specified 2D vectors.
// $ \mathbf{A}=\mathbf{B} $
static inline bool tm_vec2_equal(tm_vec2_t lhs, tm_vec2_t rhs);

// Checks whether the two specified 2D vectors are equal within a specified range `eps`.
// $ \mathbf{A}\approx\mathbf{B} $
static inline bool tm_vec2_equal_abs_eps(tm_vec2_t lhs, tm_vec2_t rhs, float eps);

// Performs linear interpolation between `a` and `b` with fraction `t`.
// $ \mathbf{C}=\mathbf{A}(1-t)+\mathbf{B}t $
static inline tm_vec2_t tm_vec2_lerp(tm_vec2_t a, tm_vec2_t b, float t);

// Performs piecewise linear interpolation between `a` and `b` with fraction `t`.
// $ \mathbf{C}=\mathbf{A}(1-\mathbf{T})+\mathbf{B}\mathbf{T} $
static inline tm_vec2_t tm_vec2_element_lerp(tm_vec2_t a, tm_vec2_t b, tm_vec2_t t);

// Returns the piecewise minimum of `a` and `b`.
static inline tm_vec2_t tm_vec2_min(tm_vec2_t a, tm_vec2_t b);

// Returns the piecewise maximum of `a` and `b`.
static inline tm_vec2_t tm_vec2_max(tm_vec2_t a, tm_vec2_t b);

// Restricts `v` to the range [`lo`, `hi`].
static inline tm_vec2_t tm_vec2_clamp(tm_vec2_t v, tm_vec2_t lo, tm_vec2_t hi);

// Calculates the squared distance from point `a` to point `b`.
// $ (\mathbf{A}-\mathbf{B})\cdot(\mathbf{A}-\mathbf{B}) $
static inline float tm_vec2_dist_sqr(tm_vec2_t a, tm_vec2_t b);

// Calculates the distance from point `a` to point `b`.
// $ \sqrt{(\mathbf{A}-\mathbf{B})\cdot(\mathbf{A}-\mathbf{B})} $
static inline float tm_vec2_dist(tm_vec2_t a, tm_vec2_t b);

// Returns the piecewise absolute value of `v`.
// $ \left|\mathbf{V}\right|=(\left|v_x\right|,\left|v_y\right|) $
static inline tm_vec2_t tm_vec2_abs(tm_vec2_t v);

// Vec3

static inline tm_vec3_t tm_vec3_add(tm_vec3_t lhs, tm_vec3_t rhs);

static inline tm_vec3_t tm_vec3_sub(tm_vec3_t lhs, tm_vec3_t rhs);

static inline float tm_vec3_dot(tm_vec3_t v1, tm_vec3_t v2);

static inline tm_vec3_t tm_vec3_cross(tm_vec3_t lhs, tm_vec3_t rhs);

static inline tm_vec3_t tm_vec3_mul(tm_vec3_t lhs, float rhs);

static inline tm_vec3_t tm_vec3_mul_add(tm_vec3_t lhs, tm_vec3_t rhs, float mul);

static inline tm_vec3_t tm_vec3_element_mul(tm_vec3_t lhs, tm_vec3_t rhs);

static inline tm_vec3_t tm_vec3_element_div(tm_vec3_t lhs, tm_vec3_t rhs);

static inline float tm_vec3_length(tm_vec3_t v);

static inline tm_vec3_t tm_vec3_normalize(tm_vec3_t);

static inline bool tm_vec3_equal(tm_vec3_t lhs, tm_vec3_t rhs);

static inline bool tm_vec3_equal_abs_eps(tm_vec3_t lhs, tm_vec3_t rhs, float eps);

static inline tm_vec3_t tm_vec3_lerp(tm_vec3_t a, tm_vec3_t b, float t);

static inline tm_vec3_t tm_vec3_element_lerp(tm_vec3_t a, tm_vec3_t b, tm_vec3_t t);

static inline tm_vec3_t tm_vec3_min(tm_vec3_t a, tm_vec3_t b);

static inline tm_vec3_t tm_vec3_max(tm_vec3_t a, tm_vec3_t b);

static inline tm_vec3_t tm_vec3_clamp(tm_vec3_t v, tm_vec3_t lo, tm_vec3_t hi);

static inline float tm_vec3_dist_sqr(tm_vec3_t a, tm_vec3_t b);

static inline float tm_vec3_dist(tm_vec3_t a, tm_vec3_t b);

static inline tm_vec3_t tm_vec3_abs(tm_vec3_t v);

// Vec4

static inline tm_vec4_t tm_vec4_add(tm_vec4_t lhs, tm_vec4_t rhs);

static inline tm_vec4_t tm_vec4_sub(tm_vec4_t lhs, tm_vec4_t rhs);

static inline float tm_vec4_dot(tm_vec4_t v1, tm_vec4_t v2);

static inline tm_vec4_t tm_vec4_mul(tm_vec4_t lhs, float rhs);

static inline tm_vec4_t tm_vec4_mul_add(tm_vec4_t lhs, tm_vec4_t rhs, float mul);

static inline tm_vec4_t tm_vec4_element_mul(tm_vec4_t lhs, tm_vec4_t rhs);

static inline tm_vec4_t tm_vec4_element_div(tm_vec4_t lhs, tm_vec4_t rhs);

static inline float tm_vec4_length(tm_vec4_t v);

static inline tm_vec4_t tm_vec4_normalize(tm_vec4_t);

static inline bool tm_vec4_equal(tm_vec4_t lhs, tm_vec4_t rhs);

static inline bool tm_vec4_equal_abs_eps(tm_vec4_t lhs, tm_vec4_t rhs, float eps);

static inline tm_vec4_t tm_vec4_lerp(tm_vec4_t a, tm_vec4_t b, float t);

static inline tm_vec4_t tm_vec4_element_lerp(tm_vec4_t a, tm_vec4_t b, tm_vec4_t t);

static inline tm_vec4_t tm_vec4_min(tm_vec4_t a, tm_vec4_t b);

static inline tm_vec4_t tm_vec4_max(tm_vec4_t a, tm_vec4_t b);

static inline tm_vec4_t tm_vec4_clamp(tm_vec4_t v, tm_vec4_t lo, tm_vec4_t hi);

static inline float tm_vec4_dist_sqr(tm_vec4_t a, tm_vec4_t b);

static inline float tm_vec4_dist(tm_vec4_t a, tm_vec4_t b);

static inline tm_vec4_t tm_vec4_abs(tm_vec4_t v);

// Mat44

// Returns the identity matrix.
// $ \begin{bmatrix}1&0&0&0\\0&1&0&0\\0&0&1&0\\0&0&0&1\end{bmatrix} $
static inline const tm_mat44_t *tm_mat44_identity();

// Creates a matrix transformation that translates any point by offset `t`.
// $ \begin{bmatrix}1&0&0&0\\0&1&0&0\\0&0&1&0\\t_x&t_y&t_z&1\end{bmatrix} $
static inline void tm_mat44_from_translation(tm_mat44_t *res, tm_vec3_t t);

// Creates a matrix transformation that scales any point by `s`.
// $ \begin{bmatrix}s_x&0&0&0\\0&s_y&0&0\\0&0&s_z&0\\0&0&0&1\end{bmatrix} $
static inline void tm_mat44_from_scale(tm_mat44_t *res, tm_vec3_t s);

// Creates a matrix transformation that rotates any point using quaternion `q`.
static inline void tm_mat44_from_quaternion(tm_mat44_t *res, tm_vec4_t q);

// Creates a matrix that defines a local to world transformation using
// translation `t`, orientation `q`, and scale `s`.
static inline void tm_mat44_from_translation_quaternion_scale(tm_mat44_t *res, tm_vec3_t t, tm_vec4_t q, tm_vec3_t s);

// Multiplies two matrices together, multiplication is non-commutative.
// Note: Matrices in The Machinery are row-major.
static inline void tm_mat44_mul(tm_mat44_t *res, const tm_mat44_t *lhs, const tm_mat44_t *rhs);

// Transposes the specified matrix `m`.
// $ \begin{bmatrix}a&b&c&d\\e&f&g&h\\i&j&k&l\\m&n&o&p\end{bmatrix}^T=\begin{bmatrix}a&e&i&m\\b&f&j&n\\c&g&k&o\\d&h&l&p\end{bmatrix} $
static inline void tm_mat44_transpose(tm_mat44_t *res, const tm_mat44_t *m);

// Computes the inverse of the specified matrix `m`.
static inline void tm_mat44_inverse(tm_mat44_t *res, const tm_mat44_t *m);

// Transforms the point `v` with the specified matrix `m` (`w` is assumed to be one).
// $ \mathbf{M}\begin{bmatrix}x&y&z&1\end{bmatrix} $
static inline tm_vec3_t tm_mat44_transform(const tm_mat44_t *m, tm_vec3_t v);

// Transforms the vector `v` with the specified matrix `m` (`w` is assumed to be zero).
// $ \mathbf{M}\begin{bmatrix}x&y&z&0\end{bmatrix} $
static inline tm_vec3_t tm_mat44_transform_no_translation(const tm_mat44_t *m, tm_vec3_t v);

// Transforms the vector `v` with the specified matrix `m`.
// $ \mathbf{M}\vec{V} $
static inline tm_vec4_t tm_mat44_transform_vec4(const tm_mat44_t *m, tm_vec4_t v);

// Computes the determinant of the specified matrix `m`.
// This is the signed area of the parallelotope defined by `m`.
static inline float tm_mat44_determinant(const tm_mat44_t *m);

// Computes the determinant of the minor matrix without the `w` row and column.
// This is the signed area of the parallopiped defined by `m`.
static inline float tm_mat44_determinant33(const tm_mat44_t *m);

// Computes a quaternion that defines the orientation property of the specified matrix `m`.
static inline tm_vec4_t tm_mat44_to_quaternion(const tm_mat44_t *m);

// Computes the transformation properties of the specified matrix `m`.
// Translation is returned in `t`, rotation in `r`, and scale in `s`.
static inline void tm_mat44_to_translation_quaternion_scale(tm_vec3_t *t, tm_vec4_t *r, tm_vec3_t *s, const tm_mat44_t *m);

// Checks whether two matrices are equal within a specified range `eps`.
static inline bool tm_mat44_equal_abs_eps(const tm_mat44_t *lhs, const tm_mat44_t *rhs, float eps);

// Returns a pointer to the unit vector for the X-axis.
static inline tm_vec3_t *tm_mat44_x(tm_mat44_t *m);

// Returns a pointer to the unit vector for the Y-axis.
static inline tm_vec3_t *tm_mat44_y(tm_mat44_t *m);

// Returns a pointer to the unit vector for the Z-axis.
static inline tm_vec3_t *tm_mat44_z(tm_mat44_t *m);

// Returns a pointer to the unit vector for the W-axis.
static inline tm_vec3_t *tm_mat44_w(tm_mat44_t *m);

// Returns a pointer to the unit vector for the X-axis.
static inline tm_vec4_t *tm_mat44_x_vec4(tm_mat44_t *m);

// Returns a pointer to the unit vector for the Y-axis.
static inline tm_vec4_t *tm_mat44_y_vec4(tm_mat44_t *m);

// Returns a pointer to the unit vector for the Z-axis.
static inline tm_vec4_t *tm_mat44_z_vec4(tm_mat44_t *m);

// Returns a pointer to the unit vector for the W-axis.
static inline tm_vec4_t *tm_mat44_w_vec4(tm_mat44_t *m);

// Quaternion

// Creates a quaternion that defines a rotation of and `a` (in radians) around the specified `axis`.
static inline tm_vec4_t tm_quaternion_from_rotation(tm_vec3_t axis, float a);

// Returns the axis around which the specfied quaternion defines its rotation,
// also returns the angle (in radians) in `a`.
static inline tm_vec3_t tm_quaternion_to_rotation(tm_vec4_t q, float *a);

// Multiplies two quaternions together. Note this is non-commutative.
static inline tm_vec4_t tm_quaternion_mul(tm_vec4_t lhs, tm_vec4_t rhs);

// Returns a quaternion that defines the inverse rotation of the specified quaternion `q`.
static inline tm_vec4_t tm_quaternion_inverse(tm_vec4_t q);

// Rotates the specified vector `v` using quaternion `q`.
static inline tm_vec3_t tm_quaternion_rotate_vec3(tm_vec4_t q, tm_vec3_t v);

// Performs normalized-linear-interpolation between quaternion `a` and `b` using fraction `t`.
// Note that nlerp is commutative and does no maintain a constant velocity.
static inline tm_vec4_t tm_quaternion_nlerp(tm_vec4_t a, tm_vec4_t b, float t);

// Converts the specified quaternion to Euler angles (roll/pitch/yaw).
static inline tm_vec3_t tm_quaternion_to_euler(tm_vec4_t q);

// Creates a quaternion from the specified Euler angles (roll/pitch/yaw).
static inline tm_vec4_t tm_quaternion_from_euler(tm_vec3_t xyz);

// Creates a quaternion that rotates the Z unit vector into the `forward` direction with the specified `up` vector.
static inline tm_vec4_t tm_quaternion_from_direction(tm_vec3_t forward, tm_vec3_t up);

// Returns the unit axis from the specified quaternion `q`. All axes are optional.
static inline void tm_quaternion_to_xyz(tm_vec3_t *x, tm_vec3_t *y, tm_vec3_t *z, tm_vec4_t q);

// uint32_t

// Returns the number of set bits in `v`.
static inline uint32_t tm_uint32_count_bits(uint32_t v);

// Returns the number of trailing zero bits in `v`.
static inline uint32_t tm_uint32_count_trailing_zero_bits(uint32_t v);

// Returns the number of leading zero bits in `v`.
static inline uint32_t tm_uint32_count_leading_zero_bits(uint32_t v);

// Returns the rounded down 2-logarithm of `v`. This is equal to the bit position of the highest
// non-zero bit in `v`. `tm_uint32_log(0)` returns `0`.
static inline uint32_t tm_uint32_log(uint32_t v);

// Rounds `v` up to the next power of 2.
static inline uint32_t tm_uint32_round_up_to_power_of_two(uint32_t v);

// Rounds `v` up to the specified power of 2 `p`.
static inline uint32_t tm_uint32_align_to_power_of_two(uint32_t v, uint32_t p);

// Computes the division `v/d`, rounding up the result to the nearest integer value.
static inline uint32_t tm_uint32_div_ceil(uint32_t v, uint32_t d);

// uint64_t

// Returns the rounded down 2-logarithm of `v`. This is equal to the bit position of the highest
// non-zero bit in `v`. `tm_uint32_log(0)` returns `0`.
static inline uint32_t tm_uint64_log(uint64_t v);

// Rounds `v` up to the specified power of 2 `p`.
static inline uint64_t tm_uint64_align_to_power_of_two(uint64_t v, uint64_t p);

// Computes the division `v/d`, rounding up the result to the nearest integer value.
static inline uint64_t tm_uint64_div_ceil(uint64_t v, uint64_t d);

// float

// Linearly interpolates from `s` to `e` a fraction `t` [0,1].
static inline float tm_lerp(float s, float e, float t);

// Computes the fraction `t` that produces the interpolates value `v` in range [`s`, `e`].
static inline float tm_inv_lerp(float s, float e, float v);

// Linearly remaps `v` from range [`s0`, `e0`] to range [`s1`, `e1`].
static inline float tm_remap(float s0, float e0, float s1, float e1, float v);

// Compare floats for equality using an absolute EPSILON test. (I.e. the magnitudes of `a` and `b`
// are not taken into account in the comparison.)
static inline bool tm_equal_abs_eps(float a, float b, float eps);

// tm_docgen off

// ----------------------------------------------------------------------------
// vector2
// ----------------------------------------------------------------------------

static inline float tm_vec2_dot(tm_vec2_t v0, tm_vec2_t v1)
{
    return v0.x * v1.x + v0.y * v1.y;
}

static inline float tm_vec2_cross(tm_vec2_t lhs, tm_vec2_t rhs)
{
    return lhs.x * rhs.y - lhs.y * rhs.x;
}

static inline float tm_vec2_length(tm_vec2_t v)
{
    return sqrtf(tm_vec2_dot(v, v));
}

static inline tm_vec2_t tm_vec2_normalize(tm_vec2_t v)
{
    const float l = tm_vec2_length(v);
    if (l < 0.00001f)
        return TM_LITERAL(tm_vec2_t){ 0 };
    const float inv_l = 1.f / l;
    return TM_LITERAL(tm_vec2_t){ v.x * inv_l, v.y * inv_l };
}

static inline tm_vec2_t tm_vec2_mul(tm_vec2_t lhs, float rhs)
{
    return TM_LITERAL(tm_vec2_t){ lhs.x * rhs, lhs.y * rhs };
}

static inline tm_vec2_t tm_vec2_mul_add(tm_vec2_t lhs, tm_vec2_t rhs, float mul)
{
    return TM_LITERAL(tm_vec2_t){ lhs.x + rhs.x * mul, lhs.y + rhs.y * mul };
}

static inline tm_vec2_t tm_vec2_element_mul(tm_vec2_t lhs, tm_vec2_t rhs)
{
    return TM_LITERAL(tm_vec2_t){ lhs.x * rhs.x, lhs.y * rhs.y };
}

static inline tm_vec2_t tm_vec2_element_div(tm_vec2_t lhs, tm_vec2_t rhs)
{
    return TM_LITERAL(tm_vec2_t){ lhs.x / rhs.x, lhs.y / rhs.y };
}

static inline tm_vec2_t tm_vec2_add(tm_vec2_t lhs, tm_vec2_t rhs)
{
    return TM_LITERAL(tm_vec2_t){ lhs.x + rhs.x, lhs.y + rhs.y };
}

static inline tm_vec2_t tm_vec2_sub(tm_vec2_t lhs, tm_vec2_t rhs)
{
    return TM_LITERAL(tm_vec2_t){ lhs.x - rhs.x, lhs.y - rhs.y };
}

static inline bool tm_vec2_equal(tm_vec2_t lhs, tm_vec2_t rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

static inline bool tm_vec2_equal_abs_eps(tm_vec2_t lhs, tm_vec2_t rhs, float eps)
{
    return tm_equal_abs_eps(lhs.x, rhs.x, eps) && tm_equal_abs_eps(lhs.y, rhs.y, eps);
}

static inline tm_vec2_t tm_vec2_lerp(tm_vec2_t a, tm_vec2_t b, float t)
{
    return tm_vec2_mul_add(tm_vec2_mul(a, 1 - t), b, t);
}

static inline tm_vec2_t tm_vec2_element_lerp(tm_vec2_t a, tm_vec2_t b, tm_vec2_t t)
{
    return TM_LITERAL(tm_vec2_t){ tm_lerp(a.x, b.x, t.x), tm_lerp(a.y, b.y, t.y) };
}

static inline tm_vec2_t tm_vec2_min(tm_vec2_t a, tm_vec2_t b)
{
    return TM_LITERAL(tm_vec2_t){ a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y };
}

static inline tm_vec2_t tm_vec2_max(tm_vec2_t a, tm_vec2_t b)
{
    return TM_LITERAL(tm_vec2_t){ a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y };
}

static inline tm_vec2_t tm_vec2_clamp(tm_vec2_t v, tm_vec2_t lo, tm_vec2_t hi)
{
    return TM_LITERAL(tm_vec2_t){
        v.x < lo.x ? lo.x : v.x > hi.x ? hi.x : v.x,
        v.y < lo.y ? lo.y : v.y > hi.y ? hi.y : v.y
    };
}

static inline float tm_vec2_dist_sqr(tm_vec2_t a, tm_vec2_t b)
{
    const tm_vec2_t sub = tm_vec2_sub(a, b);
    return tm_vec2_dot(sub, sub);
}

static inline float tm_vec2_dist(tm_vec2_t a, tm_vec2_t b)
{
    return sqrtf(tm_vec2_dist_sqr(a, b));
}

static inline tm_vec2_t tm_vec2_abs(tm_vec2_t v)
{
    return TM_LITERAL(tm_vec2_t){ fabsf(v.x), fabsf(v.y) };
}

// ----------------------------------------------------------------------------
// vector3
// ----------------------------------------------------------------------------

static inline float tm_vec3_dot(tm_vec3_t v0, tm_vec3_t v1)
{
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

static inline float tm_vec3_length(tm_vec3_t v)
{
    return sqrtf(tm_vec3_dot(v, v));
}

static inline tm_vec3_t tm_vec3_normalize(tm_vec3_t v)
{
    const float l = tm_vec3_length(v);
    if (l < 0.00001f)
        return TM_LITERAL(tm_vec3_t){ 0 };
    const float inv_l = 1.f / l;
    return TM_LITERAL(tm_vec3_t){
        v.x * inv_l,
        v.y * inv_l,
        v.z * inv_l,
    };
}

static inline tm_vec3_t tm_vec3_mul(tm_vec3_t lhs, float rhs)
{
    return TM_LITERAL(tm_vec3_t){
        lhs.x * rhs,
        lhs.y * rhs,
        lhs.z * rhs,
    };
}

static inline tm_vec3_t tm_vec3_mul_add(tm_vec3_t lhs, tm_vec3_t rhs, float mul)
{
    return TM_LITERAL(tm_vec3_t){
        lhs.x + rhs.x * mul,
        lhs.y + rhs.y * mul,
        lhs.z + rhs.z * mul,
    };
}

static inline tm_vec3_t tm_vec3_element_mul(tm_vec3_t lhs, tm_vec3_t rhs)
{
    return TM_LITERAL(tm_vec3_t){
        lhs.x * rhs.x,
        lhs.y * rhs.y,
        lhs.z * rhs.z,
    };
}

static inline tm_vec3_t tm_vec3_element_div(tm_vec3_t lhs, tm_vec3_t rhs)
{
    return TM_LITERAL(tm_vec3_t){
        lhs.x / rhs.x,
        lhs.y / rhs.y,
        lhs.z / rhs.z,
    };
}

static inline tm_vec3_t tm_vec3_add(tm_vec3_t lhs, tm_vec3_t rhs)
{
    return TM_LITERAL(tm_vec3_t){
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
    };
}

static inline tm_vec3_t tm_vec3_sub(tm_vec3_t lhs, tm_vec3_t rhs)
{
    return TM_LITERAL(tm_vec3_t){
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
    };
}

static inline tm_vec3_t tm_vec3_cross(tm_vec3_t lhs, tm_vec3_t rhs)
{
    return TM_LITERAL(tm_vec3_t){
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

static inline bool tm_vec3_equal(tm_vec3_t lhs, tm_vec3_t rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

static inline bool tm_vec3_equal_abs_eps(tm_vec3_t lhs, tm_vec3_t rhs, float eps)
{
    return tm_equal_abs_eps(lhs.x, rhs.x, eps) && tm_equal_abs_eps(lhs.y, rhs.y, eps) && tm_equal_abs_eps(lhs.z, rhs.z, eps);
}

static inline tm_vec3_t tm_vec3_lerp(tm_vec3_t a, tm_vec3_t b, float t)
{
    return tm_vec3_mul_add(tm_vec3_mul(a, 1 - t), b, t);
}

static inline tm_vec3_t tm_vec3_element_lerp(tm_vec3_t a, tm_vec3_t b, tm_vec3_t t)
{
    return TM_LITERAL(tm_vec3_t){
        tm_lerp(a.x, b.x, t.x),
        tm_lerp(a.y, b.y, t.y),
        tm_lerp(a.z, b.z, t.z)
    };
}

static inline tm_vec3_t tm_vec3_min(tm_vec3_t a, tm_vec3_t b)
{
    return TM_LITERAL(tm_vec3_t){
        a.x < b.x ? a.x : b.x,
        a.y < b.y ? a.y : b.y,
        a.z < b.z ? a.z : b.z
    };
}

static inline tm_vec3_t tm_vec3_max(tm_vec3_t a, tm_vec3_t b)
{
    return TM_LITERAL(tm_vec3_t){
        a.x > b.x ? a.x : b.x,
        a.y > b.y ? a.y : b.y,
        a.z > b.z ? a.z : b.z
    };
}

static inline tm_vec3_t tm_vec3_clamp(tm_vec3_t v, tm_vec3_t lo, tm_vec3_t hi)
{
    return TM_LITERAL(tm_vec3_t){
        v.x < lo.x ? lo.x : v.x > hi.x ? hi.x : v.x,
        v.y < lo.y ? lo.y : v.y > hi.y ? hi.y : v.y,
        v.z < lo.z ? lo.z : v.z > hi.z ? hi.z : v.z
    };
}

static inline float tm_vec3_dist_sqr(tm_vec3_t a, tm_vec3_t b)
{
    const tm_vec3_t sub = tm_vec3_sub(a, b);
    return tm_vec3_dot(sub, sub);
}

static inline float tm_vec3_dist(tm_vec3_t a, tm_vec3_t b)
{
    return sqrtf(tm_vec3_dist_sqr(a, b));
}

static inline tm_vec3_t tm_vec3_abs(tm_vec3_t v)
{
    return TM_LITERAL(tm_vec3_t){ fabsf(v.x), fabsf(v.y), fabsf(v.z) };
}

// ----------------------------------------------------------------------------
// vector4
// ----------------------------------------------------------------------------

static inline float tm_vec4_dot(tm_vec4_t lhs, tm_vec4_t rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

static inline float tm_vec4_length(tm_vec4_t v)
{
    return sqrtf(tm_vec4_dot(v, v));
}

static inline tm_vec4_t tm_vec4_normalize(tm_vec4_t v)
{
    const float l = tm_vec4_length(v);
    if (l < 0.00001f)
        return TM_LITERAL(tm_vec4_t){ 0 };
    const float inv_l = 1.f / l;
    return TM_LITERAL(tm_vec4_t){
        v.x * inv_l,
        v.y * inv_l,
        v.z * inv_l,
        v.w * inv_l,
    };
}

static inline tm_vec4_t tm_vec4_mul(tm_vec4_t lhs, float rhs)
{
    return TM_LITERAL(tm_vec4_t){
        lhs.x * rhs,
        lhs.y * rhs,
        lhs.z * rhs,
        lhs.w * rhs,
    };
}

static inline tm_vec4_t tm_vec4_mul_add(tm_vec4_t lhs, tm_vec4_t rhs, float mul)
{
    return TM_LITERAL(tm_vec4_t){
        lhs.x + rhs.x * mul,
        lhs.y + rhs.y * mul,
        lhs.z + rhs.z * mul,
        lhs.w + rhs.w * mul,
    };
}

static inline tm_vec4_t tm_vec4_element_mul(tm_vec4_t lhs, tm_vec4_t rhs)
{
    return TM_LITERAL(tm_vec4_t){
        lhs.x * rhs.x,
        lhs.y * rhs.y,
        lhs.z * rhs.z,
        lhs.w * rhs.w,
    };
}

static inline tm_vec4_t tm_vec4_element_div(tm_vec4_t lhs, tm_vec4_t rhs)
{
    return TM_LITERAL(tm_vec4_t){
        lhs.x / rhs.x,
        lhs.y / rhs.y,
        lhs.z / rhs.z,
        lhs.w / rhs.w,
    };
}

static inline tm_vec4_t tm_vec4_add(tm_vec4_t lhs, tm_vec4_t rhs)
{
    return TM_LITERAL(tm_vec4_t){
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
        lhs.w + rhs.w,
    };
}

static inline tm_vec4_t tm_vec4_sub(tm_vec4_t lhs, tm_vec4_t rhs)
{
    return TM_LITERAL(tm_vec4_t){
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
        lhs.w - rhs.w,
    };
}

static inline bool tm_vec4_equal(tm_vec4_t lhs, tm_vec4_t rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

static inline bool tm_vec4_equal_abs_eps(tm_vec4_t lhs, tm_vec4_t rhs, float eps)
{
    return tm_equal_abs_eps(lhs.x, rhs.x, eps) && tm_equal_abs_eps(lhs.y, rhs.y, eps) && tm_equal_abs_eps(lhs.z, rhs.z, eps) && tm_equal_abs_eps(lhs.w, rhs.w, eps);
}

static inline tm_vec4_t tm_vec4_lerp(tm_vec4_t a, tm_vec4_t b, float t)
{
    return tm_vec4_mul_add(tm_vec4_mul(a, 1 - t), b, t);
}

static inline tm_vec4_t tm_vec4_element_lerp(tm_vec4_t a, tm_vec4_t b, tm_vec4_t t)
{
    return TM_LITERAL(tm_vec4_t){
        tm_lerp(a.x, b.x, t.x),
        tm_lerp(a.y, b.y, t.y),
        tm_lerp(a.z, b.z, t.z),
        tm_lerp(a.w, b.w, t.w)
    };
}

static inline tm_vec4_t tm_vec4_min(tm_vec4_t a, tm_vec4_t b)
{
    return TM_LITERAL(tm_vec4_t){
        a.x < b.x ? a.x : b.x,
        a.y < b.y ? a.y : b.y,
        a.z < b.z ? a.z : b.z,
        a.w < b.w ? a.w : b.w
    };
}

static inline tm_vec4_t tm_vec4_max(tm_vec4_t a, tm_vec4_t b)
{
    return TM_LITERAL(tm_vec4_t){
        a.x > b.x ? a.x : b.x,
        a.y > b.y ? a.y : b.y,
        a.z > b.z ? a.z : b.z,
        a.w > b.w ? a.w : b.w
    };
}

static inline tm_vec4_t tm_vec4_clamp(tm_vec4_t v, tm_vec4_t lo, tm_vec4_t hi)
{
    return TM_LITERAL(tm_vec4_t){
        v.x < lo.x ? lo.x : v.x > hi.x ? hi.x : v.x,
        v.y < lo.y ? lo.y : v.y > hi.y ? hi.y : v.y,
        v.z < lo.z ? lo.z : v.z > hi.z ? hi.z : v.z,
        v.w < lo.w ? lo.w : v.w > hi.w ? hi.w : v.w
    };
}

static inline float tm_vec4_dist_sqr(tm_vec4_t a, tm_vec4_t b)
{
    const tm_vec4_t sub = tm_vec4_sub(a, b);
    return tm_vec4_dot(sub, sub);
}

static inline float tm_vec4_dist(tm_vec4_t a, tm_vec4_t b)
{
    return sqrtf(tm_vec4_dist_sqr(a, b));
}

static inline tm_vec4_t tm_vec4_abs(tm_vec4_t v)
{
    return TM_LITERAL(tm_vec4_t){ fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w) };
}

// ----------------------------------------------------------------------------
// matrix44
// ----------------------------------------------------------------------------

static inline const tm_mat44_t *tm_mat44_identity(void)
{
    static tm_mat44_t id = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    return &id;
}

static inline void tm_mat44_from_translation(tm_mat44_t *res, tm_vec3_t t)
{
    memset(res, 0, sizeof(*res));
    res->xx = 1.f;
    res->yy = 1.f;
    res->zz = 1.f;
    res->ww = 1.f;
    res->wx = t.x;
    res->wy = t.y;
    res->wz = t.z;
}

static inline void tm_mat44_from_scale(tm_mat44_t *res, tm_vec3_t s)
{
    *res = *tm_mat44_identity();
    res->xx = s.x;
    res->yy = s.y;
    res->zz = s.z;
}

static inline void tm_mat44_from_quaternion(tm_mat44_t *res, tm_vec4_t q)
{
    const float d = tm_vec4_dot(q, q);
    const float s = (d != 0.f) ? 2.f / d : 1.f;

    const float xs = q.x * s;
    const float ys = q.y * s;
    const float zs = q.z * s;
    const float wx = q.w * xs;
    const float wy = q.w * ys;
    const float wz = q.w * zs;
    const float xx = q.x * xs;
    const float xy = q.x * ys;
    const float xz = q.x * zs;
    const float yy = q.y * ys;
    const float yz = q.y * zs;
    const float zz = q.z * zs;

    res->xx = 1.f - yy - zz;
    res->xy = xy + wz;
    res->xz = xz - wy;
    res->xw = 0.f;

    res->yx = xy - wz;
    res->yy = 1.f - xx - zz;
    res->yz = yz + wx;
    res->yw = 0.f;

    res->zx = xz + wy;
    res->zy = yz - wx;
    res->zz = 1.f - xx - yy;
    res->zw = 0.f;

    res->wx = 0;
    res->wy = 0;
    res->wz = 0;
    res->ww = 1.f;
}

static inline void tm_mat44_from_translation_quaternion_scale(tm_mat44_t *res, tm_vec3_t t, tm_vec4_t q, tm_vec3_t s)
{
    tm_mat44_from_quaternion(res, q);

    res->xx *= s.x;
    res->xy *= s.x;
    res->xz *= s.x;

    res->yx *= s.y;
    res->yy *= s.y;
    res->yz *= s.y;

    res->zx *= s.z;
    res->zy *= s.z;
    res->zz *= s.z;

    res->wx = t.x;
    res->wy = t.y;
    res->wz = t.z;
}

#if defined(TM_CPU_AVX)

// Relies on AVX for splatting (_mm_broadcast_ss).
//
// TODO:
//
//  * Since this is an AVX specific implementation, maybe it should require front-aligning of the
//    matrices so we can do aligned loads and stores.
//  * Does this function really provide any speed improvement over the SSE version? See discussion
//    at <https://stackoverflow.com/questions/13218391/is-mm-broadcast-ss-faster-than-mm-set1-ps>.
//    We should investigate this.
static inline void tm_mat44_mul_avx(tm_mat44_t *res, const tm_mat44_t *lhs, const tm_mat44_t *rhs)
{
    __m128 vx = _mm_loadu_ps(&rhs->xx);
    __m128 vy = _mm_loadu_ps(&rhs->yx);
    __m128 vz = _mm_loadu_ps(&rhs->zx);
    __m128 vw = _mm_loadu_ps(&rhs->wx);

    // row 0
    __m128 sx = _mm_broadcast_ss(&lhs->xx);
    __m128 sy = _mm_broadcast_ss(&lhs->xy);
    __m128 sz = _mm_broadcast_ss(&lhs->xz);
    __m128 sw = _mm_broadcast_ss(&lhs->xw);

    sx = _mm_mul_ps(sx, vx);
    sy = _mm_mul_ps(sy, vy);
    sz = _mm_mul_ps(sz, vz);
    sw = _mm_mul_ps(sw, vw);

    sx = _mm_add_ps(sx, sz);
    sy = _mm_add_ps(sy, sw);
    sx = _mm_add_ps(sx, sy);

    _mm_storeu_ps(&res->xx, sx);

    // row 1
    sx = _mm_broadcast_ss(&lhs->yx);
    sy = _mm_broadcast_ss(&lhs->yy);
    sz = _mm_broadcast_ss(&lhs->yz);
    sw = _mm_broadcast_ss(&lhs->yw);

    sx = _mm_mul_ps(sx, vx);
    sy = _mm_mul_ps(sy, vy);
    sz = _mm_mul_ps(sz, vz);
    sw = _mm_mul_ps(sw, vw);

    sx = _mm_add_ps(sx, sz);
    sy = _mm_add_ps(sy, sw);
    sx = _mm_add_ps(sx, sy);

    _mm_storeu_ps(&res->yx, sx);

    // row 2
    sx = _mm_broadcast_ss(&lhs->zx);
    sy = _mm_broadcast_ss(&lhs->zy);
    sz = _mm_broadcast_ss(&lhs->zz);
    sw = _mm_broadcast_ss(&lhs->zw);

    sx = _mm_mul_ps(sx, vx);
    sy = _mm_mul_ps(sy, vy);
    sz = _mm_mul_ps(sz, vz);
    sw = _mm_mul_ps(sw, vw);

    sx = _mm_add_ps(sx, sz);
    sy = _mm_add_ps(sy, sw);
    sx = _mm_add_ps(sx, sy);

    _mm_storeu_ps(&res->zx, sx);

    // row 3
    sx = _mm_broadcast_ss(&lhs->wx);
    sy = _mm_broadcast_ss(&lhs->wy);
    sz = _mm_broadcast_ss(&lhs->wz);
    sw = _mm_broadcast_ss(&lhs->ww);

    sx = _mm_mul_ps(sx, vx);
    sy = _mm_mul_ps(sy, vy);
    sz = _mm_mul_ps(sz, vz);
    sw = _mm_mul_ps(sw, vw);

    sx = _mm_add_ps(sx, sz);
    sy = _mm_add_ps(sy, sw);
    sx = _mm_add_ps(sx, sy);

    _mm_storeu_ps(&res->wx, sx);
}

#endif

#ifdef TM_CPU_SSE

// Standard implementation of tm_mat44_mul (uses SSE, but doesn't require AVX).
static inline void tm_mat44_mul(tm_mat44_t *res, const tm_mat44_t *lhs, const tm_mat44_t *rhs)
{
    __m128 vx = _mm_loadu_ps(&rhs->xx);
    __m128 vy = _mm_loadu_ps(&rhs->yx);
    __m128 vz = _mm_loadu_ps(&rhs->zx);
    __m128 vw = _mm_loadu_ps(&rhs->wx);

    // row 0
    __m128 sx = _mm_set1_ps(lhs->xx);
    __m128 sy = _mm_set1_ps(lhs->xy);
    __m128 sz = _mm_set1_ps(lhs->xz);
    __m128 sw = _mm_set1_ps(lhs->xw);

    sx = _mm_mul_ps(sx, vx);
    sy = _mm_mul_ps(sy, vy);
    sz = _mm_mul_ps(sz, vz);
    sw = _mm_mul_ps(sw, vw);

    sx = _mm_add_ps(sx, sz);
    sy = _mm_add_ps(sy, sw);
    sx = _mm_add_ps(sx, sy);

    _mm_storeu_ps(&res->xx, sx);

    // row 1
    sx = _mm_set1_ps(lhs->yx);
    sy = _mm_set1_ps(lhs->yy);
    sz = _mm_set1_ps(lhs->yz);
    sw = _mm_set1_ps(lhs->yw);

    sx = _mm_mul_ps(sx, vx);
    sy = _mm_mul_ps(sy, vy);
    sz = _mm_mul_ps(sz, vz);
    sw = _mm_mul_ps(sw, vw);

    sx = _mm_add_ps(sx, sz);
    sy = _mm_add_ps(sy, sw);
    sx = _mm_add_ps(sx, sy);

    _mm_storeu_ps(&res->yx, sx);

    // row 2
    sx = _mm_set1_ps(lhs->zx);
    sy = _mm_set1_ps(lhs->zy);
    sz = _mm_set1_ps(lhs->zz);
    sw = _mm_set1_ps(lhs->zw);

    sx = _mm_mul_ps(sx, vx);
    sy = _mm_mul_ps(sy, vy);
    sz = _mm_mul_ps(sz, vz);
    sw = _mm_mul_ps(sw, vw);

    sx = _mm_add_ps(sx, sz);
    sy = _mm_add_ps(sy, sw);
    sx = _mm_add_ps(sx, sy);

    _mm_storeu_ps(&res->zx, sx);

    // row 3
    sx = _mm_set1_ps(lhs->wx);
    sy = _mm_set1_ps(lhs->wy);
    sz = _mm_set1_ps(lhs->wz);
    sw = _mm_set1_ps(lhs->ww);

    sx = _mm_mul_ps(sx, vx);
    sy = _mm_mul_ps(sy, vy);
    sz = _mm_mul_ps(sz, vz);
    sw = _mm_mul_ps(sw, vw);

    sx = _mm_add_ps(sx, sz);
    sy = _mm_add_ps(sy, sw);
    sx = _mm_add_ps(sx, sy);

    _mm_storeu_ps(&res->wx, sx);
}

#else

static inline void tm_mat44_mul(tm_mat44_t *res, const tm_mat44_t *lhs, const tm_mat44_t *rhs)
{
    *res = TM_LITERAL(tm_mat44_t){
        lhs->xx * rhs->xx + lhs->xy * rhs->yx + lhs->xz * rhs->zx + lhs->xw * rhs->wx,
        lhs->xx * rhs->xy + lhs->xy * rhs->yy + lhs->xz * rhs->zy + lhs->xw * rhs->wy,
        lhs->xx * rhs->xz + lhs->xy * rhs->yz + lhs->xz * rhs->zz + lhs->xw * rhs->wz,
        lhs->xx * rhs->xw + lhs->xy * rhs->yw + lhs->xz * rhs->zw + lhs->xw * rhs->ww,
        lhs->yx * rhs->xx + lhs->yy * rhs->yx + lhs->yz * rhs->zx + lhs->yw * rhs->wx,
        lhs->yx * rhs->xy + lhs->yy * rhs->yy + lhs->yz * rhs->zy + lhs->yw * rhs->wy,
        lhs->yx * rhs->xz + lhs->yy * rhs->yz + lhs->yz * rhs->zz + lhs->yw * rhs->wz,
        lhs->yx * rhs->xw + lhs->yy * rhs->yw + lhs->yz * rhs->zw + lhs->yw * rhs->ww,
        lhs->zx * rhs->xx + lhs->zy * rhs->yx + lhs->zz * rhs->zx + lhs->zw * rhs->wx,
        lhs->zx * rhs->xy + lhs->zy * rhs->yy + lhs->zz * rhs->zy + lhs->zw * rhs->wy,
        lhs->zx * rhs->xz + lhs->zy * rhs->yz + lhs->zz * rhs->zz + lhs->zw * rhs->wz,
        lhs->zx * rhs->xw + lhs->zy * rhs->yw + lhs->zz * rhs->zw + lhs->zw * rhs->ww,
        lhs->wx * rhs->xx + lhs->wy * rhs->yx + lhs->wz * rhs->zx + lhs->ww * rhs->wx,
        lhs->wx * rhs->xy + lhs->wy * rhs->yy + lhs->wz * rhs->zy + lhs->ww * rhs->wy,
        lhs->wx * rhs->xz + lhs->wy * rhs->yz + lhs->wz * rhs->zz + lhs->ww * rhs->wz,
        lhs->wx * rhs->xw + lhs->wy * rhs->yw + lhs->wz * rhs->zw + lhs->ww * rhs->ww,
    };
}

#endif

static inline void tm_mat44_transpose(tm_mat44_t *res, const tm_mat44_t *m)
{
    *res = TM_LITERAL(tm_mat44_t){
        m->xx, m->yx, m->zx, m->wx,
        m->xy, m->yy, m->zy, m->wy,
        m->xz, m->yz, m->zz, m->wz,
        m->xw, m->yw, m->zw, m->ww
    };
}

#if defined(TM_CPU_SSE)

// copy paste from: https://github.com/niswegmann/small-matrix-inverse/blob/master/invert4x4_sse.h
// Creative Common
static inline void tm_mat44_inverse(tm_mat44_t *res44, const tm_mat44_t *m44)
{
    float *res = &res44->xx;
    const float *m = &m44->xx;

    __m128 minor0, minor1, minor2, minor3;
    __m128 row0, row1, row2, row3;
    __m128 det, tmp1;

    tmp1 = _mm_set_ps(0, 0, 0, 0);
    row1 = _mm_set_ps(0, 0, 0, 0);
    row3 = _mm_set_ps(0, 0, 0, 0);

    tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64 *)(m)), (__m64 *)(m + 4));
    row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64 *)(m + 8)), (__m64 *)(m + 12));

    row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
    row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);

    tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64 *)(m + 2)), (__m64 *)(m + 6));
    row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64 *)(m + 10)), (__m64 *)(m + 14));

    row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
    row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);

    tmp1 = _mm_mul_ps(row2, row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor0 = _mm_mul_ps(row1, tmp1);
    minor1 = _mm_mul_ps(row0, tmp1);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
    minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
    minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);

    tmp1 = _mm_mul_ps(row1, row2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
    minor3 = _mm_mul_ps(row0, tmp1);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
    minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
    minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);

    tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    row2 = _mm_shuffle_ps(row2, row2, 0x4E);

    minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
    minor2 = _mm_mul_ps(row0, tmp1);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
    minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
    minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);

    tmp1 = _mm_mul_ps(row0, row1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
    minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));

    tmp1 = _mm_mul_ps(row0, row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
    minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
    minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));

    tmp1 = _mm_mul_ps(row0, row2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
    minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);

    det = _mm_mul_ps(row0, minor0);
    det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
    det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);

    tmp1 = _mm_rcp_ss(det);

    det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
    det = _mm_shuffle_ps(det, det, 0x00);

    minor0 = _mm_mul_ps(det, minor0);
    _mm_storel_pi((__m64 *)(res), minor0);
    _mm_storeh_pi((__m64 *)(res + 2), minor0);

    minor1 = _mm_mul_ps(det, minor1);
    _mm_storel_pi((__m64 *)(res + 4), minor1);
    _mm_storeh_pi((__m64 *)(res + 6), minor1);

    minor2 = _mm_mul_ps(det, minor2);
    _mm_storel_pi((__m64 *)(res + 8), minor2);
    _mm_storeh_pi((__m64 *)(res + 10), minor2);

    minor3 = _mm_mul_ps(det, minor3);
    _mm_storel_pi((__m64 *)(res + 12), minor3);
    _mm_storeh_pi((__m64 *)(res + 14), minor3);
}

#else

static inline void tm_mat44_inverse(tm_mat44_t *res44, const tm_mat44_t *m44)
{
    const float zwzw = m44->zz * m44->ww - m44->zw * m44->wz;
    const float ywzw = m44->zy * m44->ww - m44->zw * m44->wy;
    const float yzzw = m44->zy * m44->wz - m44->zz * m44->wy;
    const float xwzw = m44->zx * m44->ww - m44->zw * m44->wx;
    const float xzzw = m44->zx * m44->wz - m44->zz * m44->wx;
    const float xyzw = m44->zx * m44->wy - m44->zy * m44->wx;
    const float zwyw = m44->yz * m44->ww - m44->yw * m44->wz;
    const float ywyw = m44->yy * m44->ww - m44->yw * m44->wy;
    const float yzyw = m44->yy * m44->wz - m44->yz * m44->wy;
    const float zwyz = m44->yz * m44->zw - m44->yw * m44->zz;
    const float ywyz = m44->yy * m44->zw - m44->yw * m44->zy;
    const float yzyz = m44->yy * m44->zz - m44->yz * m44->zy;
    const float xwyw = m44->yx * m44->ww - m44->yw * m44->wx;
    const float xzyw = m44->yx * m44->wz - m44->yz * m44->wx;
    const float xwyz = m44->yx * m44->zw - m44->yw * m44->zx;
    const float xzyz = m44->yx * m44->zz - m44->yz * m44->zx;
    const float xyyw = m44->yx * m44->wy - m44->yy * m44->wx;
    const float xyyz = m44->yx * m44->zy - m44->yy * m44->zx;

    float det = m44->xx * (m44->yy * zwzw - m44->yz * ywzw + m44->yw * yzzw)
        - m44->xy * (m44->yx * zwzw - m44->yz * xwzw + m44->yw * xzzw)
        + m44->xz * (m44->yx * ywzw - m44->yy * xwzw + m44->yw * xyzw)
        - m44->xw * (m44->yx * yzzw - m44->yy * xzzw + m44->yz * xyzw);
    det = 1.0f / det;

    *res44 = TM_LITERAL(tm_mat44_t){
        det * (m44->yy * zwzw - m44->yz * ywzw + m44->yw * yzzw),
        det * -(m44->xy * zwzw - m44->xz * ywzw + m44->xw * yzzw),
        det * (m44->xy * zwyw - m44->xz * ywyw + m44->xw * yzyw),
        det * -(m44->xy * zwyz - m44->xz * ywyz + m44->xw * yzyz),
        det * -(m44->yx * zwzw - m44->yz * xwzw + m44->yw * xzzw),
        det * (m44->xx * zwzw - m44->xz * xwzw + m44->xw * xzzw),
        det * -(m44->xx * zwyw - m44->xz * xwyw + m44->xw * xzyw),
        det * (m44->xx * zwyz - m44->xz * xwyz + m44->xw * xzyz),
        det * (m44->yx * ywzw - m44->yy * xwzw + m44->yw * xyzw),
        det * -(m44->xx * ywzw - m44->xy * xwzw + m44->xw * xyzw),
        det * (m44->xx * ywyw - m44->xy * xwyw + m44->xw * xyyw),
        det * -(m44->xx * ywyz - m44->xy * xwyz + m44->xw * xyyz),
        det * -(m44->yx * yzzw - m44->yy * xzzw + m44->yz * xyzw),
        det * (m44->xx * yzzw - m44->xy * xzzw + m44->xz * xyzw),
        det * -(m44->xx * yzyw - m44->xy * xzyw + m44->xz * xyyw),
        det * (m44->xx * yzyz - m44->xy * xzyz + m44->xz * xyyz),
    };
}

#endif

static inline tm_vec3_t tm_mat44_transform(const tm_mat44_t *m, tm_vec3_t v)
{
    return TM_LITERAL(tm_vec3_t){
        m->xx * v.x + m->yx * v.y + m->zx * v.z + m->wx,
        m->xy * v.x + m->yy * v.y + m->zy * v.z + m->wy,
        m->xz * v.x + m->yz * v.y + m->zz * v.z + m->wz,
    };
}

static inline tm_vec3_t tm_mat44_transform_no_translation(const tm_mat44_t *m, tm_vec3_t v)
{
    return TM_LITERAL(tm_vec3_t){
        m->xx * v.x + m->yx * v.y + m->zx * v.z,
        m->xy * v.x + m->yy * v.y + m->zy * v.z,
        m->xz * v.x + m->yz * v.y + m->zz * v.z,
    };
}

static inline tm_vec4_t tm_mat44_transform_vec4(const tm_mat44_t *m, tm_vec4_t v)
{
    return TM_LITERAL(tm_vec4_t){
        m->xx * v.x + m->yx * v.y + m->zx * v.z + v.w * m->wx,
        m->xy * v.x + m->yy * v.y + m->zy * v.z + v.w * m->wy,
        m->xz * v.x + m->yz * v.y + m->zz * v.z + v.w * m->wz,
        m->xw * v.x + m->yw * v.y + m->zw * v.z + v.w * m->ww,
    };
}

static inline float tm_mat44_determinant(const tm_mat44_t *m)
{
    float det = 0;
    det += m->xw * m->yz * m->zy * m->wx - m->xz * m->yw * m->zy * m->wx - m->xw * m->yy * m->zz * m->wx + m->xy * m->yw * m->zz * m->wx;
    det += m->xz * m->yy * m->zw * m->wx - m->xy * m->yz * m->zw * m->wx - m->xw * m->yz * m->zx * m->wy + m->xz * m->yw * m->zx * m->wy;
    det += m->xw * m->yx * m->zz * m->wy - m->xx * m->yw * m->zz * m->wy - m->xz * m->yx * m->zw * m->wy + m->xx * m->yz * m->zw * m->wy;
    det += m->xw * m->yy * m->zx * m->wz - m->xy * m->yw * m->zx * m->wz - m->xw * m->yx * m->zy * m->wz + m->xx * m->yw * m->zy * m->wz;
    det += m->xy * m->yx * m->zw * m->wz - m->xx * m->yy * m->zw * m->wz - m->xz * m->yy * m->zx * m->ww + m->xy * m->yz * m->zx * m->ww;
    det += m->xz * m->yx * m->zy * m->ww - m->xx * m->yz * m->zy * m->ww - m->xy * m->yx * m->zz * m->ww + m->xx * m->yy * m->zz * m->ww;
    return det;
}

static inline float tm_mat44_determinant33(const tm_mat44_t *m)
{
    float det = 0;
    det += m->xx * m->yy * m->zz - m->xx * m->yz * m->zy;
    det += m->xy * m->yz * m->zx - m->xy * m->yx * m->zz;
    det += m->xz * m->yx * m->zy - m->xz * m->yy * m->zx;
    return det;
}

// reference code from https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
// Converting a Rotation Matrix to a Quaternion, Mike Day @ Insomniac
static inline tm_vec4_t tm_mat44_to_quaternion(const tm_mat44_t *m)
{
    const float m00 = m->xx, m10 = m->yx, m20 = m->zx;
    const float m01 = m->xy, m11 = m->yy, m21 = m->zy;
    const float m02 = m->xz, m12 = m->yz, m22 = m->zz;

    float t;
    tm_vec4_t res;
    if (m22 < 0.f) {
        if (m00 > m11) {
            t = 1.f + m00 - m11 - m22;
            res.x = t;
            res.y = m01 + m10;
            res.z = m20 + m02;
            res.w = m12 - m21;
        } else {
            t = 1.f - m00 + m11 - m22;
            res.x = m01 + m10;
            res.y = t;
            res.z = m12 + m21;
            res.w = m20 - m02;
        }
    } else {
        if (m00 < -m11) {
            t = 1.f - m00 - m11 + m22;
            res.x = m20 + m02;
            res.y = m12 + m21;
            res.z = t;
            res.w = m01 - m10;
        } else {
            t = 1.f + m00 + m11 + m22;
            res.x = m12 - m21;
            res.y = m20 - m02;
            res.z = m01 - m10;
            res.w = t;
        }
    }

    float s = 0.5f / sqrtf(t);
    res.x *= s;
    res.y *= s;
    res.z *= s;
    res.w *= s;

    return res;
}

static inline void tm_mat44_to_translation_quaternion_scale(tm_vec3_t *t, tm_vec4_t *r, tm_vec3_t *s, const tm_mat44_t *m)
{
    memcpy(t, &m->wx, sizeof(*t));

    s->x = sqrtf(m->xx * m->xx + m->xy * m->xy + m->xz * m->xz);
    s->y = sqrtf(m->yx * m->yx + m->yy * m->yy + m->yz * m->yz);
    s->z = sqrtf(m->zx * m->zx + m->zy * m->zy + m->zz * m->zz);

    tm_mat44_t tmp;
    memcpy(&tmp, m, sizeof(tmp));
    tmp.xx *= 1.f / s->x, tmp.xy *= 1.f / s->x, tmp.xz *= 1.f / s->x;
    tmp.yx *= 1.f / s->y, tmp.yy *= 1.f / s->y, tmp.yz *= 1.f / s->y;
    tmp.zx *= 1.f / s->z, tmp.zy *= 1.f / s->z, tmp.zz *= 1.f / s->z;

    bool is_mirrored = tm_mat44_determinant33(&tmp) < 0.0f;
    if (!is_mirrored) {
        *r = tm_mat44_to_quaternion(&tmp);
    } else {
        s->x = -s->x;
        s->y = -s->y;
        s->z = -s->z;

        tm_mat44_t mirror = {
            -tmp.xx, -tmp.xy, -tmp.xz, tmp.xw,
            -tmp.yx, -tmp.yy, -tmp.yz, tmp.yw,
            -tmp.zx, -tmp.zy, -tmp.zz, tmp.zw,
            +tmp.wx, +tmp.wy, +tmp.wz, tmp.ww
        };
        *r = tm_mat44_to_quaternion(&mirror);
    }
}

#if defined(TM_CPU_SSE)

static inline bool tm_mat44_equal_abs_eps(const tm_mat44_t *lhs, const tm_mat44_t *rhs, float eps)
{
    const __m128 abs_mask = _mm_castsi128_ps(_mm_srli_epi32(_mm_set1_epi32(-1), 1));
    const __m128 eps128 = _mm_set1_ps(eps);

    __m128 row = _mm_and_ps(abs_mask, _mm_sub_ps(_mm_loadu_ps(&lhs->xx), _mm_loadu_ps(&rhs->xx)));
    __m128 res = _mm_cmpge_ps(row, eps128);

    row = _mm_and_ps(abs_mask, _mm_sub_ps(_mm_loadu_ps(&lhs->yx), _mm_loadu_ps(&rhs->yx)));
    res = _mm_or_ps(res, _mm_cmpge_ps(row, eps128));

    row = _mm_and_ps(abs_mask, _mm_sub_ps(_mm_loadu_ps(&lhs->zx), _mm_loadu_ps(&rhs->zx)));
    res = _mm_or_ps(res, _mm_cmpge_ps(row, eps128));

    row = _mm_and_ps(abs_mask, _mm_sub_ps(_mm_loadu_ps(&lhs->wx), _mm_loadu_ps(&rhs->wx)));
    res = _mm_or_ps(res, _mm_cmpge_ps(row, eps128));

    return _mm_movemask_ps(res) == 0;
}

#else

static inline bool tm_mat44_equal_abs_eps(const tm_mat44_t *lhs, const tm_mat44_t *rhs, float eps)
{
    const float *lhs_16 = &lhs->xx;
    const float *rhs_16 = &rhs->xx;
    for (int i = 0; i < 16; ++i) {
        if (fabsf(lhs_16[i] - rhs_16[i]) > eps) {
            return false;
        }
    }
    return true;
}

#endif

static inline tm_vec3_t *tm_mat44_x(tm_mat44_t *m)
{
    return (tm_vec3_t *)&m->xx;
}
static inline tm_vec3_t *tm_mat44_y(tm_mat44_t *m) { return (tm_vec3_t *)&m->yx; }
static inline tm_vec3_t *tm_mat44_z(tm_mat44_t *m) { return (tm_vec3_t *)&m->zx; }
static inline tm_vec3_t *tm_mat44_w(tm_mat44_t *m) { return (tm_vec3_t *)&m->wx; }
static inline tm_vec4_t *tm_mat44_x_vec4(tm_mat44_t *m) { return (tm_vec4_t *)&m->xx; }
static inline tm_vec4_t *tm_mat44_y_vec4(tm_mat44_t *m) { return (tm_vec4_t *)&m->yx; }
static inline tm_vec4_t *tm_mat44_z_vec4(tm_mat44_t *m) { return (tm_vec4_t *)&m->zx; }
static inline tm_vec4_t *tm_mat44_w_vec4(tm_mat44_t *m) { return (tm_vec4_t *)&m->wx; }

// ----------------------------------------------------------------------------
// quaternion
// ----------------------------------------------------------------------------

static inline tm_vec4_t tm_quaternion_from_rotation(tm_vec3_t axis, float a)
{
    const float halfa = a * 0.5f;
    const float cosha = cosf(halfa);
    const float sinha = sinf(halfa);
    return TM_LITERAL(tm_vec4_t){
        axis.x * sinha,
        axis.y * sinha,
        axis.z * sinha,
        cosha,
    };
}

static inline tm_vec3_t tm_quaternion_to_rotation(tm_vec4_t q, float *a)
{
    const tm_vec3_t v = { q.x, q.y, q.z };
    const float s = tm_vec3_length(v);
    *a = 2.0f * atan2f(s, q.w);
    return s ? tm_vec3_mul(v, 1 / s) : TM_LITERAL(tm_vec3_t){ 1, 0, 0 };
}

static inline tm_vec4_t tm_quaternion_mul(tm_vec4_t lhs, tm_vec4_t rhs)
{
    return TM_LITERAL(tm_vec4_t){
        lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x,
        lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
    };
}

static inline tm_vec4_t tm_quaternion_inverse(tm_vec4_t q)
{
    return TM_LITERAL(tm_vec4_t){
        -q.x,
        -q.y,
        -q.z,
        q.w,
    };
}

static inline tm_vec3_t tm_quaternion_rotate_vec3(tm_vec4_t q, tm_vec3_t v)
{
    const tm_vec4_t v4 = { v.x, v.y, v.z, 0 };
    const tm_vec4_t q_inv = tm_quaternion_inverse(q);
    const tm_vec4_t v4_rot = tm_quaternion_mul(q, tm_quaternion_mul(v4, q_inv));
    return TM_LITERAL(tm_vec3_t){ v4_rot.x, v4_rot.y, v4_rot.z };
}

static inline tm_vec4_t tm_quaternion_nlerp(tm_vec4_t a, tm_vec4_t b, float t)
{
    // Make sure quaternions have same sign
    const float dot = tm_vec4_dot(a, b);
    const tm_vec4_t q = tm_vec4_mul_add(tm_vec4_mul(a, 1 - t), b, dot >= 0 ? t : -t);
    return tm_vec4_normalize(q);
}

static inline tm_vec3_t tm_quaternion_to_euler(tm_vec4_t q)
{
    const float sinr = 2 * (q.w * q.x + q.y * q.z);
    const float cosr = 1 - 2 * (q.x * q.x + q.y * q.y);
    const float roll = atan2f(sinr, cosr);

    const float sinp = 2 * (q.w * q.y - q.z * q.x);
    const float pitch = sinp >= 0.999f ? TM_PI / 2 : sinp <= -0.999f ? -TM_PI / 2 : asinf(sinp);

    const float siny = 2 * (q.w * q.z + q.x * q.y);
    const float cosy = 1 - 2 * (q.y * q.y + q.z * q.z);
    const float yaw = atan2f(siny, cosy);
    return TM_LITERAL(tm_vec3_t){ roll, pitch, yaw };
}

static inline tm_vec4_t tm_quaternion_from_euler(tm_vec3_t xyz)
{
    const float roll = xyz.x;
    const float pitch = xyz.y;
    const float yaw = xyz.z;

    const float cy = cosf(yaw * 0.5f);
    const float sy = sinf(yaw * 0.5f);
    const float cr = cosf(roll * 0.5f);
    const float sr = sinf(roll * 0.5f);
    const float cp = cosf(pitch * 0.5f);
    const float sp = sinf(pitch * 0.5f);

    return TM_LITERAL(tm_vec4_t){
        cy * sr * cp - sy * cr * sp,
        cy * cr * sp + sy * sr * cp,
        sy * cr * cp - cy * sr * sp,
        cy * cr * cp + sy * sr * sp,
    };
}

static inline tm_vec4_t tm_quaternion_from_direction(tm_vec3_t forward, tm_vec3_t up)
{
    const tm_vec3_t right = tm_vec3_normalize(tm_vec3_cross(up, forward));
    const float t = right.x + up.y + forward.z;

    tm_vec4_t res;
    if (t > 0.0f) {
        float l = sqrtf(t + 1.0f);
        res.w = l * 0.5f;
        l = 0.5f / l;

        res.x = (up.z - forward.y) * l;
        res.y = (forward.x - right.z) * l;
        res.z = (right.y - up.x) * l;
    } else if (right.x >= up.y && right.x >= forward.z) {
        float l = sqrtf(1.0f - right.x - up.y - forward.z);
        res.x = l * 0.5f;
        l = 0.5f / l;

        res.y = (right.y + up.x) * l;
        res.z = (right.z + forward.x) * l;
        res.w = (up.z - forward.y) * l;
    } else if (up.y > forward.z) {
        float l = sqrtf(1.0f + up.y - right.x - forward.z);
        res.y = l * 0.5f;
        l = 0.5f / l;

        res.x = (up.x + right.y) * l;
        res.z = (forward.y + up.z) * l;
        res.w = (forward.x - right.z) * l;
    } else {
        float l = sqrtf(1.0f + forward.z - right.x - up.y);
        res.z = 0.5f * l;
        l = 0.5f / l;

        res.x = (forward.x + right.z) * l;
        res.y = (forward.y + up.z) * l;
        res.w = (right.y - up.x) * l;
    }

    return res;
}

static inline void tm_quaternion_to_xyz(tm_vec3_t *x, tm_vec3_t *y, tm_vec3_t *z, tm_vec4_t q)
{
    const float d = tm_vec4_dot(q, q);
    const float s = (d != 0.f) ? 2.f / d : 1.f;

    const float xs = q.x * s;
    const float ys = q.y * s;
    const float zs = q.z * s;
    const float wx = q.w * xs;
    const float wy = q.w * ys;
    const float wz = q.w * zs;
    const float xx = q.x * xs;
    const float xy = q.x * ys;
    const float xz = q.x * zs;
    const float yy = q.y * ys;
    const float yz = q.y * zs;
    const float zz = q.z * zs;

    if (x) {
        x->x = 1.f - yy - zz;
        x->y = xy + wz;
        x->z = xz - wy;
    }

    if (y) {
        y->x = xy - wz;
        y->y = 1.f - xx - zz;
        y->z = yz + wx;
    }

    if (z) {
        z->x = xz + wy;
        z->y = yz - wx;
        z->z = 1.f - xx - yy;
    }
}

// ----------------------------------------------------------------------------
// uint32
// ----------------------------------------------------------------------------

// X86 / Windows intrinsics: http://www.felixcloutier.com/x86/
// Fallbacks: http://graphics.stanford.edu/~seander/bithacks.html

static inline uint32_t tm_uint32_count_bits(uint32_t v)
{
#if defined(_MSC_VER) && !defined(__clang__)
    return _mm_popcnt_u32(v);
#else
    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    return (((v + (v >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
#endif
}

static inline uint32_t tm_uint32_count_trailing_zero_bits(uint32_t v)
{
#if defined(_MSC_VER) && !defined(__clang__)
    return _tzcnt_u32(v);
#else
    uint32_t c = 32;
    v &= -(signed)(v);
    if (v)
        c--;
    if (v & 0x0000FFFF)
        c -= 16;
    if (v & 0x00FF00FF)
        c -= 8;
    if (v & 0x0F0F0F0F)
        c -= 4;
    if (v & 0x33333333)
        c -= 2;
    if (v & 0x55555555)
        c -= 1;
    return c;
#endif
}

static inline uint32_t tm_uint32_count_leading_zero_bits(uint32_t v)
{
#if defined(TM_OS_WINDOWS)
    unsigned long res = 0;
    return _BitScanReverse(&res, v) ? 31 - res : 32;
#else
    return v ? __builtin_clz(v) : 32;
#endif
}

static inline uint32_t tm_uint32_log(uint32_t v)
{
#if defined(TM_OS_WINDOWS)
    unsigned long res = 0;
    _BitScanReverse(&res, v);
    return res;
#else
    return 31 - __builtin_clz(v);
#endif
}

static inline uint32_t tm_uint32_round_up_to_power_of_two(uint32_t v)
{
    return v ? 1 << (tm_uint32_log(v - 1) + 1) : 0;
}

static inline uint32_t tm_uint32_align_to_power_of_two(uint32_t v, uint32_t p)
{
    return (v + p - 1) & ~(p - 1);
}

// Computes the integer division x/y, rounding the result up instead of down. Note: This looks more
// expensive than (x + y - 1)/y, but the compiler usually optimizes this to fewer instructions
// (both / and % are calcuated with a single div instruction).
static inline uint32_t tm_uint32_div_ceil(uint32_t v, uint32_t d)
{
    return v / d + (v % d ? 1 : 0);
}

// ----------------------------------------------------------------------------
// uint64
// ----------------------------------------------------------------------------

static inline uint32_t tm_uint64_log(uint64_t v)
{
#if defined(TM_OS_WINDOWS)
    unsigned long res = 0;
    _BitScanReverse64(&res, v);
    return res;
#else
    return 63 - __builtin_clzll(v);
#endif
}

static inline uint32_t tm_uint64_count_bits(uint64_t v)
{
#if defined(_MSC_VER) && !defined(__clang__)
    return (uint32_t)_mm_popcnt_u64(v);
#else
    v = v - ((v >> 1) & 0x5555555555555555ULL);
    v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
    return (uint32_t)((((v + (v >> 4)) & 0x0F0F0F0F0F0F0F0FULL) * 0x0101010101010101ULL) >> 56);
#endif
}

static inline uint64_t tm_uint64_align_to_power_of_two(uint64_t v, uint64_t p)
{
    return (v + p - 1) & ~(p - 1);
}

static inline uint64_t tm_uint64_div_ceil(uint64_t v, uint64_t d)
{
    return v / d + (v % d ? 1 : 0);
}

static inline float tm_lerp(float s, float e, float t)
{
    return s * (1.f - t) + e * t;
}

static inline float tm_inv_lerp(float s, float e, float v)
{
    return (v - s) / (e - s);
}

static inline float tm_remap(float s0, float e0, float s1, float e1, float v)
{
    return tm_lerp(s1, e1, tm_inv_lerp(s0, e0, v));
}

static inline bool tm_equal_abs_eps(float a, float b, float eps)
{
    return fabsf(a - b) < eps;
}


#endif