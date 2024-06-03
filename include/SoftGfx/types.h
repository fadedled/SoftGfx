/*
 * SoftGfx - 1.0 - public domain
 * types.h: Common types
 */

#ifndef __TYPES_H__
#define __TYPES_H__


#include <stdint.h>

/* Integers */
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;

/* Boolean, for convenience */
typedef int32_t   bint;

/* Floating point, for convenience */
typedef float     f32;
typedef double    f64;

/* Boolean, for convenience */
typedef int32_t   bint;

#define TRUE       1
#define FALSE      0

/* Vectors */
typedef float     vec2[2];
typedef float     vec3[3];
typedef float     vec4[4];


/* Matrix Array offsets are specified column first */
typedef float     mat4[16];
typedef float     mat3[9];


typedef struct Vert_t {
	vec3	pos;
	vec3	norm;
	vec3	color;
	vec2 	tex;
} Vert;


#endif /*__TYPES_H__*/
