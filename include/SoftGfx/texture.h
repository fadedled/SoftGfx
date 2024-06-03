/*
 * SoftGfx - 1.0 - public domain
 * texture.h: Texture related types and functions
 */

#ifndef __TEXTURE_H__
#define __TEXTURE_H__


#include <SoftGfx/types.h>


//an RBG texture
typedef struct Tex_t {
	u32		id;		//texture id
	u32		w;		//texture width
	u32		h;		//texture height
	u32		bpp;	//NUMBER OF BPP
	u8*		data;	//using 24bpp, REALLY???
} Tex;


Tex* gfxTexLoadBMP(const char *filename);
void gfxTexFree(Tex *tex);
//void gfxMeshFree(Mesh *o);

#endif /*__TEXTURE_H__*/
