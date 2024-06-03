/*
 * SoftGfx - 1.0 - public domain
 * gfx.c : Graphics related functions
 */

#include <stdio.h>
#include <SoftGfx/light.h>
#include <SoftGfx/vm_math.h>
#include <math.h>


/*Light state*/
struct LightEXT_t {
	Light	l[GFX_MAX_LIGHTS];	//Light position
	vec3	vpos[GFX_MAX_LIGHTS];
} lights;
static Material material;
static u32 light_act;


/*Sets the current material*/
void
gfxMaterialSet(Material *m)
{
	material = *m;
}

/*Avtivates the lights that will be used (lowest bit is first light and so on)*/
void
gfxLightActive(u32 active_bit)
{
	light_act = active_bit;
}

/*Updates active light position to view space*/
void
gfxLightViewUpdate(mat4 view)
{
	for (u32 i = 0; i < GFX_MAX_LIGHTS; ++i) {
		if ((light_act >> i) & 1) {
			vec3_mat4Mul(lights.vpos[i], view, lights.l[i].pos);
		}
	}
}

/*Sets the Light properties*/
void
gfxLightSet(u8 light_id, Light *l)
{
	light_id = light_id & (GFX_MAX_LIGHTS - 1);
	lights.l[light_id] = *l;
}


/*Calculates the lighting given the position and normal (also pass view positon for world view)*/
void
_gfxComputeLighting(vec3 out, vec3 pos, vec3 norm)
{
	vec3 ambient, diffuse, specular = {0.0f}, light_dir, view_dir, ref_dir, tmp;
	const vec3 VZERO = {0.0f, 0.0f, 0.0f};
	out[0] = 0.0f;
	out[1] = 0.0f;
	out[2] = 0.0f;

	vec3_normalize(norm);
	vec3_normalize(vec3_sub(view_dir, VZERO, pos));
	for (u32 i = 0; i < GFX_MAX_LIGHTS; ++i) {
		if ((light_act >> i) & 1) {
			/*Diffuse calc*/
			vec3_normalize(vec3_sub(light_dir, lights.vpos[i], pos));
			f32 diff = vec3_dot(norm, light_dir);
			diff = (diff > 0.0f ? diff : 0.0f);

			/*Specular calc*/
			vec3_reflect(ref_dir, vec3_sub(tmp, VZERO, light_dir), norm);
			f32 spec = vec3_dot(view_dir, ref_dir);
			spec = powf((spec > 0.0f ? spec : 0.0f), material.Se);

			/*Combine components*/
			vec3_mul(ambient, lights.l[i].color, material.Ka);
			vec3_mul(diffuse, lights.l[i].color, vec3_smul(diffuse, diff, material.Kd));
			vec3_mul(specular, lights.l[i].color, vec3_smul(specular, spec, material.Ks));

			/*Add to out color*/
			vec3_add(out, vec3_add(out, vec3_add(out, out, ambient), diffuse), specular);
		}
	}
}



