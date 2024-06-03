/*
 * SoftGfx - 1.0 - public domain
 * light.h: Lighting related functions
 */

#ifndef __LIGHT_H__
#define __LIGHT_H__


#include <SoftGfx/types.h>


#define GFX_MAX_LIGHTS		8


/*Struct that specifies a light*/
typedef struct Light_t {
	vec3	pos;	//Light position
	vec3	color;		//Ambient light
} Light;

/*Struct that specifies a material*/
typedef struct Material_t {
	vec3	Ka;		//Ambient component
	vec3	Kd;		//Diffuse component
	vec3	Ks;		//Specular component
	f32		Se;		//Shininess
} Material;


void gfxMaterialSet(Material *m);
void gfxLightActive(u32 active_bit);
void gfxLightViewUpdate(mat4 view);
void gfxLightSet(u8 light_id, Light *l);


#endif /*__LIGHT_H__*/
