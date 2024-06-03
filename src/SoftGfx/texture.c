/*
 * SoftGfx - 1.0 - public domain
 * texture.c: Texture related types and functions
 */


#include <SoftGfx/texture.h>
#include <SoftGfx/vm_math.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/*Dummy Function for loading texture file*/
Tex*
gfxTexLoad(const char *filename)
{
	Tex *tex = (Tex*) malloc(sizeof(Tex));
	FILE *in = fopen(filename, "r");
	if (!in) {
 		printf("ERROR: The texture file %s was not found.", filename);
 		return NULL;
	}


	fclose(in);
	return tex;
}



/*Function for loading BMP texturefiles*/
Tex*
gfxTexLoadBMP(const char *filename)
{
	Tex *tex = (Tex*) malloc(sizeof(Tex));
	FILE *in = fopen(filename, "rb");
	if (!in) {
 		printf("ERROR: The texture file %s was not found.", filename);
 		return NULL;
	}
	u32 dat_ofs;
	fseek(in, 0x000A, SEEK_SET);
	fread(&dat_ofs, 4, 1, in);
	fseek(in, 0x0012, SEEK_SET);
	fread(&(tex->w), 4, 1, in);
	fseek(in, 0x0016, SEEK_SET);
	fread(&(tex->h), 4, 1, in);
	s16 bitsPerPixel;
	fseek(in, 0x001C, SEEK_SET);
	fread(&bitsPerPixel, 2, 1, in);
	tex->bpp = ((u32)bitsPerPixel) >> 3;

	u32 row_size_padded = (u32)(4 * ceilf((f32)tex->w / 4.0f)) * tex->bpp;
	u32 row_size_unpadded = tex->w * tex->bpp;
	u32 size = row_size_unpadded * (tex->h);
	tex->data = (u8*) malloc(size);
	u32 i = 0;
	u8 *ptr = tex->data + ((tex->h - 1) * row_size_unpadded);
	for (i = 0; i < tex->h; ++i) {
		fseek(in, dat_ofs + (i * row_size_padded), SEEK_SET);
		fread(ptr, 1, row_size_unpadded, in);
		ptr -= row_size_unpadded;
	}
	fclose(in);
	return tex;
}



/*Samples the i-th texture*/
void
_gfxSampleTex(vec3 sample_out, Tex *tex, vec2 uv)
{
	vec3 c0, c1, c2, c3;

	f64 ifu, ifv;
	f64 fu = modf((f64) uv[0] * (tex->w - 1), &ifu);
	f64 fv = modf((f64) uv[1] * (tex->h - 1), &ifv);
	u32 iu = ((u32)ifu) % tex->w;
	u32 iv = ((u32)ifv) % tex->h;

	u32 iuv = (iu * tex->bpp) + (iv * tex->bpp * tex->w);
	c0[2] = ((f32) tex->data[iuv]) / 255.0f;
	c0[1] = ((f32) tex->data[iuv + 1]) / 255.0f;
	c0[0] = ((f32) tex->data[iuv + 2]) / 255.0f;

	iuv += tex->bpp;
	c1[2] = ((f32) tex->data[iuv]) / 255.0f;
	c1[1] = ((f32) tex->data[iuv + 1]) / 255.0f;
	c1[0] = ((f32) tex->data[iuv + 2]) / 255.0f;

	iuv += tex->bpp * (tex->w - 1);
	c2[2] = ((f32) tex->data[iuv]) / 255.0f;
	c2[1] = ((f32) tex->data[iuv + 1]) / 255.0f;
	c2[0] = ((f32) tex->data[iuv + 2]) / 255.0f;

	iuv += tex->bpp;
	c3[2] = ((f32) tex->data[iuv]) / 255.0f;
	c3[1] = ((f32) tex->data[iuv + 1]) / 255.0f;
	c3[0] = ((f32) tex->data[iuv + 2]) / 255.0f;

	vec3_lerp(c0, c0, c2, fv);
	vec3_lerp(c1, c1, c3, fv);
	vec3_lerp(sample_out, c0, c1, fu);

	vec3_lerp(sample_out, c0, c1, fu);
}


void
gfxTexFree(Tex *tex)
{
	free(tex);
}
