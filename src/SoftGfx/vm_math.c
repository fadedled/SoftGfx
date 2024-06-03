/*
 * SoftGfx - 1.0 - public domain
 * vm_math.c : Implements matrix and vector functions
 */

#include <string.h>
#include <math.h>
#include <SoftGfx/vm_math.h>

/* Returns the addition of vector v and u */
f32*
vec3_add(vec3 dest, const vec3 v, const vec3 u)
{
	dest[0] = v[0] + u[0];
	dest[1] = v[1] + u[1];
	dest[2] = v[2] + u[2];
	return dest;
}


/* Returns the subtraction of vector v and u */
f32*
vec3_sub(vec3 dest, const vec3 v, const vec3 u)
{
	dest[0] = v[0] - u[0];
	dest[1] = v[1] - u[1];
	dest[2] = v[2] - u[2];
	return dest;
}


/* Returns the product of vector v and u */
f32*
vec3_mul(vec3 dest, const vec3 v, const vec3 u)
{
	dest[0] = v[0] * u[0];
	dest[1] = v[1] * u[1];
	dest[2] = v[2] * u[2];
	return dest;
}


/* Returns the product of vector v by scalar k */
f32*
vec3_smul(vec3 dest, f32 k, const vec3 v)
{
	dest[0] = k * v[0];
	dest[1] = k * v[1];
	dest[2] = k * v[2];
	return dest;
}

bint
vec3_eq(const vec3 u, const vec3 v)
{
	return (u[0] == v[0]) & (u[1] == v[1]) & (u[2] == v[2]);
}

/* Clamps each component in range [a, b]*/
void
vec3_clamp(vec3 v, f32 a, f32 b)
{
	f32 b0 = ( v[0] < b ? v[0] : b);
	f32 b1 = ( v[1] < b ? v[1] : b);
	f32 b2 = ( v[2] < b ? v[2] : b);

	v[0] = ( b0 > a ? b0 : a);
	v[1] = ( b1 > a ? b1 : a);
	v[2] = ( b2 > a ? b2 : a);
}


/* Returns the per-value linear interpolation of v and u*/
void
vec3_lerp(vec3 dest, const vec3 v, const vec3 u, f32 t)
{
	dest[0] = v[0] + (t * (u[0] - v[0]));
	dest[1] = v[1] + (t * (u[1] - v[1]));
	dest[2] = v[2] + (t * (u[2] - v[2]));
}


/* Returns dot product between vector v and u */
f32
vec3_dot(const vec3 v, const vec3 u)
{
	return v[0] * u[0] + v[1] * u[1] + v[2] * u[2];
}


/* Returns cross product between vector v and u */
void
vec3_cross(vec3 n, const vec3 v, const vec3 u)
{
	n[0] = (v[1] * u[2]) - (v[2] * u[1]);
	n[1] = (v[2] * u[0]) - (v[0] * u[2]);
	n[2] = (v[0] * u[1]) - (v[1] * u[0]);
}


/* Returns refltection direction for i (N should be normalized)*/
void
vec3_reflect(vec3 dest, const vec3 i, const vec3 n)
{
	f32 dt = vec3_dot(n, i) * 2.0f;
	dest[0] = i[0] - (dt * n[0]);
	dest[1] = i[1] - (dt * n[1]);
	dest[2] = i[2] - (dt * n[2]);
}


/* Normalizes vector v */
void
vec3_normalize(vec3 v)
{
	float mag = (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
	if (mag == 1.0f || mag == 0.0f) {
		return;
	}
	//Is not normalized
	mag = 1.0f / sqrt(mag);
	v[0] *= mag;
	v[1] *= mag;
	v[2] *= mag;
}


/* Normalizes vector v */
void
vec4_normalize(vec4 v)
{
	f32 mag = (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]) + (v[3] * v[3]);
	if (mag == 1.0f || mag == 0.0f) {
		return;
	}
	/* Is not normalized */
	mag = 1.0f / sqrt(mag);
	v[0] *= mag;
	v[1] *= mag;
	v[2] *= mag;
	v[3] *= mag;
}

/* Multiplies vec3 column vector v with matrix m  and stores result in dest */
void
vec3_mat4Mul(vec3 dest, const mat4 m, vec3 p)
{
	f32 a00 = m[0], a01 = m[4], a02 = m[8],  a03 = m[12],
		a10 = m[1], a11 = m[5], a12 = m[9],  a13 = m[13],
		a20 = m[2], a21 = m[6], a22 = m[10], a23 = m[14],
		a30 = m[3], a31 = m[7], a32 = m[11], a33 = m[15],

		b00 = p[0], b01 = p[1], b02 = p[2];

	dest[0] = (a00 * b00) + (a01 * b01) + (a02 * b02) + a03;
	dest[1] = (a10 * b00) + (a11 * b01) + (a12 * b02) + a13;
	dest[2] = (a20 * b00) + (a21 * b01) + (a22 * b02) + a23;
	/* Homogenize */
	f32 w = (a30 * b00) + (a31 * b01) + (a32 * b02) + a33;

	if (w != 0.0f) {
		f32 inv = 1.0f / w;
		dest[0] *= inv;
		dest[1] *= inv;
		dest[2] *= inv;
	}
}

f32
vec3_mat4MulStandard(vec3 dest, const mat4 m, vec3 p)
{
	f32 a00 = m[0], a01 = m[4], a02 = m[8],  a03 = m[12],
		a10 = m[1], a11 = m[5], a12 = m[9],  a13 = m[13],
		a20 = m[2], a21 = m[6], a22 = m[10], a23 = m[14],
		a30 = m[3], a31 = m[7], a32 = m[11], a33 = m[15],

		b00 = p[0], b01 = p[1], b02 = p[2];

	dest[0] = (a00 * b00) + (a01 * b01) + (a02 * b02) + a03;
	dest[1] = (a10 * b00) + (a11 * b01) + (a12 * b02) + a13;
	dest[2] = (a20 * b00) + (a21 * b01) + (a22 * b02) + a23;
	/* Homogenize */
	return (a30 * b00) + (a31 * b01) + (a32 * b02) + a33;
}

/* Multiplies vec3 column vector v with matrix m  and stores result in dest */
void
vec3_matMul(vec3 dest, const mat3 m, vec3 p)
{
	f32 a00 = m[0], a01 = m[3], a02 = m[6],
		a10 = m[1], a11 = m[4], a12 = m[7],
		a20 = m[2], a21 = m[5], a22 = m[8],

		b00 = p[0], b01 = p[1], b02 = p[2];

	dest[0] = (a00 * b00) + (a01 * b01) + (a02 * b02);
	dest[1] = (a10 * b00) + (a11 * b01) + (a12 * b02);
	dest[2] = (a20 * b00) + (a21 * b01) + (a22 * b02);
}




/* Multiplies column vector v with matrix m  and stores result in dest */
void
vec4_matMul(vec4 dest, const mat4 m, vec4 p)
{
	f32 a00 = m[0], a01 = m[4], a02 = m[8],  a03 = m[12],
		a10 = m[1], a11 = m[5], a12 = m[9],  a13 = m[13],
		a20 = m[2], a21 = m[6], a22 = m[10], a23 = m[14],
		a30 = m[3], a31 = m[7], a32 = m[11], a33 = m[15],

		b00 = p[0], b01 = p[1], b02 = p[2];

	dest[0] = (a00 * b00) + (a01 * b01) + (a02 * b02) + a03;
	dest[1] = (a10 * b00) + (a11 * b01) + (a12 * b02) + a13;
	dest[2] = (a20 * b00) + (a21 * b01) + (a22 * b02) + a23;
	/* Homogenize */
	dest[3] = (a30 * b00) + (a31 * b01) + (a32 * b02) + a33;

	if (dest[3] != 0.0f) {
		f32 inv = 1.0f / dest[3];
		dest[0] *= inv;
		dest[1] *= inv;
		dest[2] *= inv;
		dest[3] = 1.0f;
	}
}


/* Makes m the identity matrix */
void
mat4_identity(mat4 m)
{
	memset(m, 0, sizeof(f32)*16);
	m[0] = m[5] = m[10] = m[15] = 1.0f;
}


/* Matrix multiplication
 * multiplies dest = m1 * m2, stores it in dest (can be same as m2 or m1)
 */
void
mat4_mul(mat4 dest, const mat4 m1, const mat4 m2)
{
	const f32
		b00 = m1[0], b01 = m1[4], b02 = m1[8],  b03 = m1[12],
		b10 = m1[1], b11 = m1[5], b12 = m1[9],  b13 = m1[13],
		b20 = m1[2], b21 = m1[6], b22 = m1[10], b23 = m1[14],
		b30 = m1[3], b31 = m1[7], b32 = m1[11], b33 = m1[15],

		a00 = m2[0], a01 = m2[4], a02 = m2[8],  a03 = m2[12],
		a10 = m2[1], a11 = m2[5], a12 = m2[9],  a13 = m2[13],
		a20 = m2[2], a21 = m2[6], a22 = m2[10], a23 = m2[14],
		a30 = m2[3], a31 = m2[7], a32 = m2[11], a33 = m2[15];

	dest[0]  = a00*b00 + a10*b01 + a20*b02 + a30*b03;
	dest[1]  = a00*b10 + a10*b11 + a20*b12 + a30*b13;
	dest[2]  = a00*b20 + a10*b21 + a20*b22 + a30*b23;
	dest[3]  = a00*b30 + a10*b31 + a20*b32 + a30*b33;

	dest[4]  = a01*b00 + a11*b01 + a21*b02 + a31*b03;
	dest[5]  = a01*b10 + a11*b11 + a21*b12 + a31*b13;
	dest[6]  = a01*b20 + a11*b21 + a21*b22 + a31*b23;
	dest[7]  = a01*b30 + a11*b31 + a21*b32 + a31*b33;

	dest[8]  = a02*b00 + a12*b01 + a22*b02 + a32*b03;
	dest[9]  = a02*b10 + a12*b11 + a22*b12 + a32*b13;
	dest[10] = a02*b20 + a12*b21 + a22*b22 + a32*b23;
	dest[11] = a02*b30 + a12*b31 + a22*b32 + a32*b33;

	dest[12] = a03*b00 + a13*b01 + a23*b02 + a33*b03;
	dest[13] = a03*b10 + a13*b11 + a23*b12 + a33*b13;
	dest[14] = a03*b20 + a13*b21 + a23*b22 + a33*b23;
	dest[15] = a03*b30 + a13*b31 + a23*b32 + a33*b33;
}


/* Calculates the normal matrix of the model matrix m */
void
mat4_normalMatrix(mat3 dest, const mat4 m)
{
	const f32
		a00 = m[0], a01 = m[4], a02 = m[8],
		a10 = m[1], a11 = m[5], a12 = m[9],
		a20 = m[2], a21 = m[6], a22 = m[10];

	f32 d = a00 * (a11 * a22 - a12 * a21)
			- a01 * (a10 * a22 - a12 * a20)
			+ a02 * (a10 * a21 - a11 * a20);
	/* check if inverse exists */
	if (!d) {
		return;
	}

	d = 1.0f / d;

	dest[0]  = d * (a11 * a22 - a12 * a21);
	dest[1]  = d * (a02 * a21 - a01 * a22);
	dest[2]  = d * (a01 * a12 - a02 * a11);
	dest[3]  = d * (a12 * a20 - a10 * a22);
	dest[4]  = d * (a00 * a22 - a02 * a20);
	dest[5]  = d * (a02 * a10 - a00 * a12);
	dest[6]  = d * (a10 * a21 - a11 * a20);
	dest[7]  = d * (a01 * a20 - a00 * a21);
	dest[8]  = d * (a00 * a11 - a01 * a10);
}


/* Applies traslation to m. */
void
mat4_translate(mat4 m, vec3 v)
{
	f32 x = v[0], y = v[1], z = v[2];
	m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
	m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
	m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
	m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];
}


/* Applies rotation to m around to vector (x y z), angle in degrees */
void
mat4_rotate(mat4 m, vec3 v, f32 angle)
{
	mat4 rot = {0};
	f32 x, y, z,
		  sn = sinf((angle * PI) / 180.0f),
		  co = cosf((angle * PI) / 180.0f),
		  rco = 1.0f - co;

	vec3_normalize(v);
	x = v[0]; y = v[1]; z = v[2];

	rot[0]  = (x * x * rco) + co;
	rot[1]  = (y * x * rco) + (z * sn);
	rot[2]  = (z * x * rco) - (y * sn);

	rot[4]  = (x * y * rco) - (z * sn);
	rot[5]  = (y * y * rco) + co;
	rot[6]  = (z * y * rco) + (x * sn);

	rot[8]  = (x * z * rco) + (y * sn);
	rot[9]  = (y * z * rco) - (x * sn);
	rot[10] = (z * z * rco) + co;
	rot[15] = 1.0f;

	mat4_mul(m, m, rot);
}


/* Applies orthographic matrix to m */
void
mat4_ortho(mat4 m, f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
	mat4 ortho = {0};
	f32	rl = 1.0f / (right - left),
			tb = 1.0f / (top - bottom),
			fn = 1.0f / (far - near);

	ortho[0]  =  2.0f * rl;
	ortho[5]  =  2.0f * tb;
	ortho[10] = -2.0f * fn;
	ortho[12] = -(right + left) * rl;
	ortho[13] = -(top + bottom) * tb;
	ortho[14] = -(far + near) * fn;
	ortho[15] =  1.0f;

	mat4_mul(m, m, ortho);
}


/* Applies scaling to m */
void
mat4_scale(mat4 m, vec3 v)
{
	f32 x = v[0], y = v[1], z = v[2];
	m[0] *= x;	m[1] *= x;	m[2]  *= x; m[3]  *= x;
	m[4] *= y;	m[5] *= y;	m[6]  *= y; m[7]  *= y;
	m[8] *= z;	m[9] *= z;	m[10] *= z; m[11] *= z;
}

/* Applies proyection matrix to m */
void
mat4_perspective(mat4 m, f32 fovy, f32 aspect, f32 znear, f32 zfar)
{
	mat4 proj = {0};
	f32 invtan = 1.0f / tanf((fovy * PI)/360.0f);

	proj[0]  = invtan / aspect;
	proj[5]  = invtan;
	proj[10] = -(zfar + znear) / (zfar - znear);
	proj[11] = -1.0f;
	proj[14] = -(zfar * znear * 2) / (zfar - znear);

	mat4_mul(m, m, proj);
}




