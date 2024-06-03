/*
 * SoftGfx - 1.0 - public domain
 * gfx.h: Graphics related functions
 */

#ifndef __GFX_H__
#define __GFX_H__


#include <SoftGfx/vm_math.h>
#include <SoftGfx/light.h>
#include <SoftGfx/mesh.h>
#include <SoftGfx/texture.h>

/*Primitive types*/
#define GFX_POINT					1
#define GFX_LINE					2
#define GFX_TRIANGLE				3

/*Defines for lighting mode*/
#define GFX_LIGHT_NONE				0
#define GFX_LIGHT_GOURAUD			1
#define GFX_LIGHT_PHONG				2

/*Defines for state settings*/
#define GFX_DEPTH_TEST				0x00
#define GFX_LIGHTING_MODE			0x01


void gfxClearColor(u8 r, u8 g, u8 b);
void gfxClear(void);
void gfxSet(u32 var, u32 value);
void gfxDisplayRect(u32 x, u32 y, u32 width, u32 height);

/* Drawing Functions */
//void gfxLineTest(f32 x1, f32 y1, f32 x2, f32 y2);
void gfxDraw(u32 prim_type, Vert *v_arr, u32 count, mat4 proj, mat4 view, mat4 model, Tex* tex);
void gfxDrawMesh(Mesh *msh, mat4 proj, mat4 view, Tex* tex);

#endif /*__GFX_H__*/
