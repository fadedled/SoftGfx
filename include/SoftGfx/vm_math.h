/*
 * SoftGfx - 1.0 - public domain
 * vm_math.h: Matrix and vector functions
 */

#ifndef __VM_MATH_H__
#define __VM_MATH_H__


#include <SoftGfx/types.h>


#define PI			3.141592654f


/* Vector related functions */
f32* vec3_add(vec3 dest, const vec3 v, const vec3 u);
f32* vec3_sub(vec3 dest, const vec3 v, const vec3 u);
f32* vec3_mul(vec3 dest, const vec3 v, const vec3 u);
f32* vec3_smul(vec3 dest, f32 k, const vec3 v);
bint vec3_eq(const vec3 u, const vec3 v);
void vec3_clamp(vec3 v, f32 a, f32 b);
void vec3_lerp(vec3 dest, const vec3 v, const vec3 u, f32 a);
f32 vec3_dot(const vec3 v, const vec3 u);
void vec3_cross(vec3 n, const vec3 v, const vec3 u);
void vec3_reflect(vec3 dest, const vec3 m, const vec3 p);
void vec3_normalize(vec3 v);
void vec3_matMul(vec3 dest, const mat3 m, vec3 p);
void vec3_mat4Mul(vec3 dest, const mat4 m, vec3 p);
f32 vec3_mat4MulStandard(vec3 dest, const mat4 m, vec3 p);
void vec4_normalize(vec4 v);
void vec4_matMul(vec4 dest, const mat4 m, vec4 p);

/* Matrix related */
void mat4_identity(mat4 m);
void mat4_mul(mat4 dest, const mat4 m1, const mat4 m2);
void mat4_normalMatrix(mat3 dest, const mat4 m);

/* Transformations */
void mat4_translate(mat4 m, vec3 v);
void mat4_rotate(mat4 m, vec3 v, f32 angle);
void mat4_scale(mat4 m, vec3 v);

/* Projections */
void mat4_ortho(mat4 m, f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
void mat4_perspective(mat4 m, f32 fovy, f32 aspect, f32 znear, f32 zfar);


#endif /*__VM_MATH_H__*/
