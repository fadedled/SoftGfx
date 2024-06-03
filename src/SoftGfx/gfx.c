/*
 * SoftGfx - 1.0 - public domain
 * gfx.c : Core graphics drawing functions
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <SoftGfx/gfx.h>


/* Triangle spans for triangle drawing */
#define SPAN_LEFT		0
#define SPAN_RIGHT		1

typedef struct span_t {
	s32 x[2];
} span;

/* Struct for storing renderer information */
static struct gfx_renderer_t {
	u32 *pix;				// CPU Screen pixels
	f32 *zbuff;				// CPU z-buffer
	u32 clear_color;		// clear color
	u32 max_w;				// screen width
	u32 max_h;				// screen height

	/*DisplayRect dimensions*/
	u32 vp_x;
	u32 vp_y;
	u32 vp_w;
	u32 vp_h;

	span *spans;			// Triangle spans
	u32 lighting_mode;
	bint depth_test;
} ren = {0x0};

//=============================================================================

/*Defines for converting from screen space to display space*/
#define PIXW(x)		((u32) (((x + 1.0f) * 0.5f * (ren.vp_w - 1)) + ren.vp_x))
#define PIXH(y)		((u32) (((1.0f - y) * 0.5f * (ren.vp_h - 1)) + ren.vp_y))

#define COORDX(fx)	(((f32) (2.0f * (fx - ren.vp_x)) / (ren.vp_w - 1)) - 1.0f)
#define COORDY(fy)	(-(((f32) (2.0f * (fy - ren.vp_y)) / (ren.vp_h - 1)) - 1.0f))

//=============================================================================
/*Linear interpolation from two floats*/
static inline f32
lerp (f32 a, f32 b, f32 t)
{
	return a + (t * (b - a));
}

/*Edge function*/
static inline f32
edge_func(f32 ax, f32 ay, f32 bx, f32 by, f32 px, f32 py) {
	return ((bx - ax) * (py - ay)) - ((by - ay) * (px - ax));
}

/*Clamps x in range [a, b]*/
static inline f32
clamp(f32 x, f32 a, f32 b)
{
	f32 res = ( x < b ? x : b);
	return (res > a ? res : a);
}


/*Obtains the RGB value of the color, ceach component must be in range [0, 1]*/
static inline u32
vec3_toRGB(vec3 color)
{
	return  ((u32) (color[0] * 255.0f)) |
			((u32) (color[1] * 255.0f) << 8)  |
			((u32) (color[2] * 255.0f) << 16);
}

/*Linear interpolates attribute of vertex*/
static inline void
vec3_lerpAttr(vec3 out, f32 w0, const vec3 v0,
						f32 w1, const vec3 v1,
						f32 w2, const vec3 v2, f32 inv_z)
{
	out[0] = ((w0 * v0[0]) + (w1 * v1[0]) + (w2 * v2[0])) * inv_z;
	out[1] = ((w0 * v0[1]) + (w1 * v1[1]) + (w2 * v2[1])) * inv_z;
	out[2] = ((w0 * v0[2]) + (w1 * v1[2]) + (w2 * v2[2])) * inv_z;
}

/*Linear interpolates attribute of vertex*/
static inline void
vec2_lerpAttr(vec2 out, f32 w0, const vec2 v0,
						f32 w1, const vec2 v1,
						f32 w2, const vec2 v2, f32 inv_z)
{
	out[0] = ((w0 * v0[0]) + (w1 * v1[0]) + (w2 * v2[0])) * inv_z;
	out[1] = ((w0 * v0[1]) + (w1 * v1[1]) + (w2 * v2[1])) * inv_z;
}


extern void _gfxComputeLighting(vec3 out, vec3 pos, vec3 norm);
void _gfxSampleTex(vec3 sample, Tex *tex, vec2 uv);

//=============================================================================


/* Initialize display pixels */
void
gfxDisplayInit(u32 width, u32 height, u32 win_width, u32 win_height)
{
	if (ren.pix != NULL) {
		free(ren.pix);
	}
	ren.pix = (u32*) calloc(width * height, sizeof(*ren.pix));
	ren.zbuff = (f32*) calloc(width * height, sizeof(*ren.zbuff));
	ren.max_w = width;
	ren.max_h = height;
	ren.spans = (span*) calloc(height, sizeof(*ren.spans));
	ren.lighting_mode = 0;
	ren.depth_test = 1;
	gfxDisplayRect(0, 0, win_width, win_height);
	if (!ren.pix) {
		printf("ERROR: Could not create display\n");
		exit(-1);
	}
}


/* Frees all display dinamic memory used */
u32*
gfxDisplayGet(void)
{
	return ren.pix;
}


/* Frees all display dinamic memory used */
void
gfxDisplayQuit(void)
{
	if (ren.pix != NULL) {
		free(ren.pix);
		free(ren.spans);
		ren.pix = NULL;
	}
}

void
gfxSet(u32 var, u32 value)
{
	switch (var) {
	case GFX_DEPTH_TEST: {
		ren.depth_test = value;
	} break;
	case GFX_LIGHTING_MODE: {
		ren.lighting_mode = value;
	} break;
	}
}



/*============================================================================*/
/* Clears the display with the clear color over the Display Rect */
void
gfxClear(void)
{
	if (ren.pix != NULL) {
		u32 *p = ren.pix + (ren.vp_y * ren.max_w) + ren.vp_x;
		u32 *pend = ren.pix + (ren.vp_h * ren.max_w) + ren.vp_w;
		/* Also Clear Z-Buffer */
		f32 *z = ren.zbuff + (ren.vp_y * ren.max_w) + ren.vp_x;
		while (p < pend) {
			u32 *pln = p + ren.vp_w;
			while (p < pln) {
				*p++ = ren.clear_color;
				*z++ = 1.0f;
			}
			z += ren.max_w - ren.vp_h;
			p += ren.max_w - ren.vp_h;
		}
	}
}


/* Sets a new clear color */
void
gfxClearColor(u8 r, u8 g, u8 b)
{
	ren.clear_color = ((u32) r << 16u) | ((u32) g << 8u) | ((u32) b);
}

/*Set the current display rect */
void
gfxDisplayRect(u32 x, u32 y, u32 width, u32 height)
{
	ren.vp_x = (ren.max_w < x ? ren.max_w : x);
	ren.vp_y = (ren.max_h < y ? ren.max_h : y);
	ren.vp_w = (ren.max_w < (ren.vp_x + width) ? ren.max_w - ren.vp_x : width);
	ren.vp_h = (ren.max_h < (ren.vp_y + height) ? ren.max_h - ren.vp_y : height);
}



/*STATIC FUNCTION FOR COMPUTING SPAN OF TRIANGLE*/
static void
_gfxSpanCompute(f32 x1, f32 y1, f32 x2, f32 y2, u32 side)
{
	f32 x = x1;
	u32 y = y1, dy = abs(y2 - y1);
	s32 sy = (y1 < y2 ? 1 : -1);
	f32 dx = (x2 - x1) / (f32) (dy > 0.0 ? dy : 1.0f);
	u32 i = 1;
	while (i <= dy) {
		/*Terrible clipping method*/
		if (y >= ren.vp_y && y < (ren.vp_y + ren.vp_h)) {
			ren.spans[y].x[side] = (u32) clamp(ceilf(x), ren.vp_x, ren.vp_x + ren.vp_w - 1);
		}
		x += dx;
		y += sy;
		i++;
	}
}


/*
 * UNUSED, WE CLIP INSIDE _triangle
u32
_glxClipTriPlane(Vert *tri, u32 count, f32 px0, f32 py0, f32 px1, f32 py1)
{
	Vert tmp[2 * 3]; // Max number of possible triangles

	//WITH X = -1.0f
	u32 inside0 = (tri[0].pos[0] >= -1.0f);
	u32 inside1 = (tri[1].pos[0] >= -1.0f);
	u32 inside2 = (tri[2].pos[0] >= -1.0f);
	u32 new_tris = 0;
	f32 x = -1.0f

	switch (inside0 + inside1 + inside2) {
		case 0: {
			new_tris = 0;
		} break;
		case 1: {
			new_tris = 1;
			//GET THE ONE THAT IS INSIDE
			u32 v0_ins = inside1 + (inside2 << 1);
			u32 v1_ins = (v0_ins + 1) % 3;
			u32 v2_ins = (v0_ins + 2) % 3;
			tmp[0] = tri[v0_ins];
			tmp[1] = tri[v1_ins];
			tmp[2] = tri[v2_ins];
			f32 t1 = (x - tmp[1].pos[0])) / (tmp[0].pos[0] - tmp[1].pos[0]);
			f32 t2 = (x - tmp[2].pos[0])) / (tmp[0].pos[0] - tmp[2].pos[0]);
			tmp[0] = tri[0];
			tmp[1].pos[0] = x;
			tmp[1].pos[1] += ((tmp[0].pos[1] - tmp[1].pos[1]) * t1;
			tmp[2].pos[0] = x;
			tmp[2].pos[1] += ((tmp[0].pos[1] - tmp[2].pos[1]) * t2;
		} break;
		case 2: {
			new_tris = 2;

		} break;
		case 3: {
			new_tris = 1;
			tmp[0] = tri[0];
			tmp[1] = tri[1];
			tmp[2] = tri[2];
		} break;
	}
	return new_tris;
}
*/

/*Draws a point*/
void
_point(Vert *p, mat4 proj)
{
	vec3 tmp;
	vec4 spv;
	f32 *sp = spv, x, y, z;

	/*Get display space positions*/
	sp[3] = 1.0f / vec3_mat4MulStandard(sp, proj, p->pos);
	vec3_smul(sp, sp[3], sp);
	x = PIXW(sp[0]), y = PIXH(sp[1]), z = sp[2];
	if (sp[0] < -1.0f || sp[0] > 1.0f ||
		sp[1] < -1.0f || sp[1] > 1.0f) {
		return;
	}
	/*Shading*/
	if (ren.lighting_mode) {
		_gfxComputeLighting(tmp, p->pos, p->norm);
		vec3_mul(p->color, tmp, p->color);
		vec3_clamp(p->color, 0.0f, 1.0f);
	}
	/*clip z in [0, 1]*/
	u32 offset = (u32)(y * ren.max_w) + (u32) x;
	f32 curr_z = ren.zbuff[offset];
	if (ren.depth_test) {
		if ((z < 0.0f || curr_z < z)) {
			return;
		}
	}
	/*Draw pixel and update z-buffer*/
	ren.zbuff[offset] = (ren.depth_test ? z : curr_z);
	ren.pix[offset] = vec3_toRGB(p->color);
}


/*Draws a triangle*/
void
_triangle(Vert *p0, Vert *p1, Vert *p2, mat4 proj, Tex *tex)
{
	Vert *tmp_swap;
	vec4 spv0, spv1, spv2;
	f32 *sp0 = spv0, *sp1 = spv1, *sp2 = spv2, *tmp_v;

	/*Screen space transform */
	sp0[3] = 1.0f / vec3_mat4MulStandard(sp0, proj, p0->pos);
	sp1[3] = 1.0f / vec3_mat4MulStandard(sp1, proj, p1->pos);
	sp2[3] = 1.0f / vec3_mat4MulStandard(sp2, proj, p2->pos);
	vec3_smul(sp0, sp0[3], sp0);
	vec3_smul(sp1, sp1[3], sp1);
	vec3_smul(sp2, sp2[3], sp2);
	/*Check if its front facing*/
	f32 area = edge_func(sp0[0], sp0[1], sp1[0], sp1[1], sp2[0], sp2[1]);
	if (!(area > 0.0f)) {
		return;
	}

	/* Order p from highest to lowest y value (p0=highest)*/
	if (sp0[1] < sp1[1]) {
		tmp_swap = p0;
		p0 = p1;
		p1 = tmp_swap;
		tmp_v = sp0;
		sp0 = sp1;
		sp1 = tmp_v;
	}
	if (sp1[1] < sp2[1]) {
		tmp_swap = p1;
		p1 = p2;
		p2 = tmp_swap;
		tmp_v = sp1;
		sp1 = sp2;
		sp2 = tmp_v;
	}
	if (sp0[1] < sp1[1]) {
		tmp_swap = p0;
		p0 = p1;
		p1 = tmp_swap;
		tmp_v = sp0;
		sp0 = sp1;
		sp1 = tmp_v;
	}
	/*Display space transform*/
	sp0[0] = PIXW(sp0[0]), sp0[1] = PIXH(sp0[1]);
	sp1[0] = PIXW(sp1[0]), sp1[1] = PIXH(sp1[1]);
	sp2[0] = PIXW(sp2[0]), sp2[1] = PIXH(sp2[1]);
	/*Compute spans of triangle depending on midle point location*/
	bint cmp = edge_func(sp0[0], sp0[1], sp2[0], sp2[1], sp1[0], sp1[1]) > 0.0;
	_gfxSpanCompute(sp0[0], sp0[1], sp1[0], sp1[1], !cmp);
	_gfxSpanCompute(sp0[0], sp0[1], sp2[0], sp2[1], cmp);
	_gfxSpanCompute(sp1[0], sp1[1], sp2[0], sp2[1], !cmp);

	/*Precompute perspective correction*/
	const f32 d = 1.0f / ((sp0[0] - sp2[0])*(sp1[1] - sp0[1]) - (sp0[0] - sp1[0])*(sp2[1] - sp0[1]));
	const vec3 bar_d0 = {(sp1[1] - sp2[1])*d, (sp2[1] - sp0[1])*d, (sp0[1] - sp1[1])*d};
	const vec3 bar_d1 = {(sp2[0] - sp1[0])*d, (sp0[0] - sp2[0])*d, (sp1[0] - sp0[0])*d};
	const vec3 bar_0  = {(sp1[0]*sp2[1] - sp2[0]*sp1[1]) * d, (sp2[0]*sp0[1] - sp0[0]*sp2[1]) * d, (sp0[0]*sp1[1] - sp1[0]*sp0[1]) * d};
	const vec3 vpos_z = {sp0[2], sp1[2], sp2[2]};
	const vec3 vpos_w = {sp0[3], sp1[3], sp2[3]};
	/*GOURAUD SHADING if active*/
	if (ren.lighting_mode == GFX_LIGHT_GOURAUD) {
		vec3 tmp;
		_gfxComputeLighting(tmp, p0->pos, p0->norm);
		vec3_mul(p0->color, tmp, p0->color);
		vec3_clamp(p0->color, 0.0f, 1.0f);
		_gfxComputeLighting(tmp, p1->pos, p1->norm);
		vec3_mul(p1->color, tmp, p1->color);
		vec3_clamp(p1->color, 0.0f, 1.0f);
		_gfxComputeLighting(tmp, p2->pos, p2->norm);
		vec3_mul(p2->color, tmp, p2->color);
		vec3_clamp(p2->color, 0.0f, 1.0f);
	}

	/*Iterate spans to render inside triangle*/
	const u32 y_begin = clamp(sp0[1], ren.vp_y, ren.vp_y + ren.vp_h - 1);
	const u32 y_end = clamp(sp2[1], ren.vp_y, ren.vp_y + ren.vp_h - 1);
	//#pragma omp parallel for
	//printf("tri: %d %d\n", y_begin, y_end);
	for (u32 y = y_begin; y < y_end; ++y) {
		vec3 pos_attr, col_attr, norm_attr, tmp;
		vec2 tex_attr;
		u32 x = ren.spans[y].x[0];
		u32 xend = ren.spans[y].x[1];
		u32 ln = (y * ren.max_w);
		for (; x < xend; ++x) {
			vec3 bar;
			f32 fx = x + 0.5f, fy = y + 0.5f;
			/*Get barycentric coordinates*/
			vec3_add(bar, vec3_smul(tmp, fx, bar_d0), bar_0);
			vec3_add(bar, bar, vec3_smul(tmp, fy, bar_d1));
			vec3_clamp(bar, 0.0f, 1.0f);
			bar[2] = 1.0f - bar[0] - bar[1];

			/*clip z in [0, 1]*/
			f32 z = vec3_dot(bar, vpos_z);
			f32 curr_z = ren.zbuff[ln + x];
			if (ren.depth_test) {
				if (z < 0.0f || curr_z < z) {
					continue;
				}
			}
			/*Precompute perspective correction for interpolation*/
			f32 w0 = bar[0] * vpos_w[0];
			f32 w1 = bar[1] * vpos_w[1];
			f32 w2 = bar[2] * vpos_w[2];
			f32 inv_p = 1.0f / (w0 + w1 + w2);

			/*interpolate face color*/
			vec3_lerpAttr(col_attr, w0, p0->color, w1, p1->color, w2, p2->color, inv_p);
			/*Apply texture to face*/
			vec2_lerpAttr(tex_attr,  w0, p0->tex, w1, p1->tex, w2, p2->tex, inv_p);
			_gfxSampleTex(tmp, tex, tex_attr);
			vec3_mul(col_attr, tmp, col_attr);

			/*PHONG SHADING if active*/
			if (ren.lighting_mode == GFX_LIGHT_PHONG) {
				vec3_lerpAttr(pos_attr, w0, p0->pos, w1, p1->pos, w2, p2->pos, inv_p);
				vec3_lerpAttr(norm_attr, w0, p0->norm, w1, p1->norm, w2, p2->norm, inv_p);
				_gfxComputeLighting(tmp, pos_attr, norm_attr);
				vec3_mul(col_attr, tmp, col_attr);
			}
			vec3_clamp(col_attr, 0.0f, 1.0f);

			/*Draw pixel and update z-buffer*/
			ren.zbuff[ln + x] = (ren.depth_test ? z : curr_z);
			ren.pix[ln + x] = vec3_toRGB(col_attr);
		}
	}
}


void
gfxDraw(u32 prim_type, Vert *v_arr, u32 count, mat4 proj, mat4 view, mat4 model, Tex *tex)
{
	mat4 mv;
	mat3 normat;

	/*Check primitive type*/
	if (prim_type - 1 > 2) {
		return;
	}
	mat4_mul(mv, view, model);
	mat4_normalMatrix(normat, mv);

	/*Obtain matrix for first camera*/
	switch(prim_type) {
	case GFX_POINT: {
		/*Draw all points*/
		for (u32 i = 0; i < count; ++i) {
			Vert p = v_arr[i];
			vec3_mat4Mul(p.pos, mv, p.pos);
			if (ren.lighting_mode) {
				vec3_matMul(p.norm, normat, p.norm);
			}
			_point(&p, proj);
		}
	} break;
	case GFX_LINE: {
		count -= count % prim_type;
		/*Draw all lines*/
		for (u32 i = 0; i < count; i += 2) {
			Vert p[2] = {v_arr[i], v_arr[i+1]};
			vec3_mat4Mul(p[0].pos, mv, p[0].pos);
			vec3_mat4Mul(p[1].pos, mv, p[1].pos);
			if (ren.lighting_mode) {
				vec3_matMul(p[0].norm, normat, p[0].norm);
				vec3_matMul(p[1].norm, normat, p[1].norm);
			}
			//TODO: IMPLEMENT LINE FUNCTION
			//_line(p, p + 1, view_pos, proj);
		}
	} break;
	case GFX_TRIANGLE: {
		count -= count % prim_type;
		/*Draw all Triangles*/
		for (u32 i = 0; i < count; i += 3) {
			Vert p[3] = {v_arr[i], v_arr[i+1], v_arr[i+2]};
			vec3_mat4Mul(p[0].pos, mv, p[0].pos);
			vec3_mat4Mul(p[1].pos, mv, p[1].pos);
			vec3_mat4Mul(p[2].pos, mv, p[2].pos);
			if (ren.lighting_mode) {
				vec3_matMul(p[0].norm, normat, p[0].norm);
				vec3_matMul(p[1].norm, normat, p[1].norm);
				vec3_matMul(p[2].norm, normat, p[2].norm);
			}
			_triangle(p, p + 1, p + 2, proj, tex);
		}
	} break;
	}
}


/*Draws the given mesh with textures and transformation matrices*/
void
gfxDrawMesh(Mesh *msh, mat4 proj, mat4 view, Tex *tex)
{
	mat4 mv;
	mat3 normat;

	gfxMaterialSet(&msh->mtrl);
	/*Check primitive type*/
	mat4_mul(mv, view, msh->model);
	mat4_normalMatrix(normat, mv);

	u32 count = msh->indx_count - (msh->indx_count % 3);
	if (count < 3) {
		return;
	}

	/*Draw all Triangles*/
	//printf("DRAW BEGIN..\n");
	for (u32 i = 0; i < count; i += 3) {
		Vert p[3] = {msh->vrtx[msh->indx[i]],
					 msh->vrtx[msh->indx[i+1]],
					 msh->vrtx[msh->indx[i+2]]};
		vec3_mat4Mul(p[0].pos, mv, p[0].pos);
		vec3_mat4Mul(p[1].pos, mv, p[1].pos);
		vec3_mat4Mul(p[2].pos, mv, p[2].pos);
		if (ren.lighting_mode) {
			vec3_matMul(p[0].norm, normat, p[0].norm);
			vec3_matMul(p[1].norm, normat, p[1].norm);
			vec3_matMul(p[2].norm, normat, p[2].norm);
		}

		_triangle(p, p + 1, p + 2, proj, tex);
	}
}
