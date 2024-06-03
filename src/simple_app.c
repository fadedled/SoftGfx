/*
 * SoftGfx - 1.0 - public domain
 * simple_app.c : Main application
 */

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SoftGfx/gfx.h>


#define DISP_W		480
#define DISP_H		480

#define KEY_UP		0x01
#define KEY_DOWN	0x02
#define KEY_LEFT	0x04
#define KEY_RIGHT	0x08

/*Enum for projection mode*/
enum {
	PROJ_ORTHO,
	PROJ_PERSP,
	PROJ_MAX,
};



/*Light constants*/
static Light light_white = {
	{-0.08f, 0.10f,  0.15f},  //position
	{0.8f, 0.8f, 0.8f}	//color
};

static Light light_blue = {
	{-0.5f, 0.20f,  -0.5f},  //position
	{ 0.1f, 0.2f, 0.8f}	//color
};


/*Material Constant*/
static Material mtrl_set[2] = {{
	/*Material A*/
	{0.0f, 0.0f, 0.0f},	//ambient
	{0.5f, 0.5f, 0.5f},	//diffuse
	{0.7f, 0.7f, 0.7f},	//specular
	32.0f	//shininess
},{
	/*Material B*/
	{0.23125f, 0.23125f, 0.23125f},	//ambient
	{0.2775f, 0.2775f, 0.2775f},	//diffuse
	{0.773911f, 0.773911f, 0.773911f},	//specular
	89.6f	//shininess
}};

/*Pointer for current mesh*/
Mesh *g_mesh;
Tex *g_tex;

/*State of the application*/
struct AppState_t {
	/*Current projection matrix*/
	Mesh mesh[2];
	Tex  *texture[2];
	/* Both camera views */
	mat4 proj;
	mat4 cam[3];
	vec3 scale[2];

	/*Flags*/
	s32 angle_x;
	s32 angle_y;
	bint has_anim;
	bint proj_mode;
	bint mesh_mode;
	bint tex_mode;
	bint mtrl_mode;

	u32	light_act;
	u32	light_mode;
	u32	camera_mat;

	u32 keys;
} state;



/* Keyboard  */
void
appKeyboard(SDL_Scancode key, u32 event)
{
	/*Get Keyboard*/
	switch (key) {
		/* Set keys for input */
		case SDL_SCANCODE_LEFT:{
			state.keys = (state.keys & ~KEY_LEFT) | (event == SDL_PRESSED ? KEY_LEFT : 0);
		} break;
		case SDL_SCANCODE_UP:{
			state.keys = (state.keys & ~KEY_UP) | (event == SDL_PRESSED ? KEY_UP : 0);
		} break;
		case SDL_SCANCODE_RIGHT:{
			state.keys = (state.keys & ~KEY_RIGHT) | (event == SDL_PRESSED ? KEY_RIGHT : 0);
		} break;
		case SDL_SCANCODE_DOWN:{
			state.keys = (state.keys & ~KEY_DOWN) | (event == SDL_PRESSED ? KEY_DOWN : 0);
		} break;
		default: break;
	}

	if (event == SDL_PRESSED) {
		switch (key) {
			/* Toggle projection mode */
			case SDL_SCANCODE_P:{
				state.proj_mode = (state.proj_mode + 1) % PROJ_MAX;
				mat4_identity(state.proj);
				if (state.proj_mode == PROJ_ORTHO) {
					mat4_ortho(state.proj, -0.2f, 0.2f, -0.2f, 0.2f, -1.0f, 1.0f);
				} else {
					mat4_perspective(state.proj, 60.0f, 1.0f, 0.01f, 10.0f);
				}
			} break;
			/*Toggle light */
			case SDL_SCANCODE_L:{
				state.light_act = (state.light_act + 1u) & 0x1u;
				gfxLightActive((state.light_act << 1) | 1);
			} break;
			/*Toggle camera matrix */
			case SDL_SCANCODE_C:{
				state.camera_mat = (state.camera_mat + 1) % 3;
			} break;
			/*Toggle mesh model*/
			case SDL_SCANCODE_M:{
				state.mesh_mode = (state.mesh_mode + 1) & 1;
				g_mesh = state.mesh + state.mesh_mode;
			} break;
			/*Toggle used texture*/
			case SDL_SCANCODE_T:{
				state.tex_mode = (state.tex_mode + 1) & 1;
				g_tex = state.texture[state.tex_mode];
			} break;
			/*Toggle used material*/
			case SDL_SCANCODE_K:{
				state.mtrl_mode = (state.mtrl_mode + 1) & 1;
				state.mesh[0].mtrl = mtrl_set[state.mtrl_mode];
				state.mesh[1].mtrl = mtrl_set[state.mtrl_mode];
			} break;
			default: break;
		}
	}
}


/* Updates state */
void
appUpdate()
{
	/*Only update if there is animation present*/
	u32 xd = ((state.keys & KEY_RIGHT) >> 3) - ((state.keys & KEY_LEFT) >> 2);
	u32 yd = (state.keys & KEY_UP) - ((state.keys & KEY_DOWN) >> 1);
	state.angle_x += xd;
	state.angle_x = (state.angle_x < 0 ? 359 : state.angle_x);
	state.angle_x = (state.angle_x >= 360 ? 0 : state.angle_x);
	state.angle_y += yd;
	state.angle_y = (state.angle_y > 90 ? 90 : state.angle_y);
	state.angle_y = (state.angle_y < -90 ? -90 : state.angle_y);
}


/* Draws to screen */
int
appDraw()
{
	vec3 up_x = {1.0f, 0.0f, 0.0f};
	vec3 up_y = {0.0f, 1.0f, 0.0f};
	/*Since the scale is off, we must scale the models*/
	vec3 scl[2] =	{{0.04f, 0.04f, 0.04f}, 	// bunny
					{0.02f, 0.02f, 0.02f}};		// statue
	/*Clear display*/
	gfxClear();
	/*Create model matrix */
	mat4_identity(g_mesh->model);
	mat4_rotate(g_mesh->model, up_x, state.angle_y);
	mat4_rotate(g_mesh->model, up_y, state.angle_x);
	mat4_scale(g_mesh->model, scl[state.mesh_mode]);
	/*Set lights*/
	gfxLightViewUpdate(state.cam[state.camera_mat]);
	/*Draw the box*/
	gfxDrawMesh(g_mesh, state.proj, state.cam[state.camera_mat], g_tex);
	return 1;
}


void
appQuit(void)
{
	/*Free mesh memory*/
	gfxMeshFree(state.mesh);
	gfxMeshFree(state.mesh+1);
	gfxTexFree(state.texture[0]);
	gfxTexFree(state.texture[1]);
}

/* Application initializer function */
void
appSetup(void)
{
	vec3 trans = {0.0f, 0.0f, -0.3f};
	vec3 upy = {0.0f, 1.0f, 0.0f};
	vec3 upx = {-1.0f, 0.0f, 0.0f};

	/*Set Perspective matrix*/
	state.proj_mode = PROJ_PERSP;
	mat4_identity(state.proj);
	mat4_perspective(state.proj, 60.0f, 1.0f, 0.01f, 10.0f);

	/*Set first Camera*/
	mat4_identity(state.cam[0]);
	mat4_translate(state.cam[0], trans);
	mat4_rotate(state.cam[0], upx, -20.0f);
	mat4_rotate(state.cam[0], upy, 60.0f);

	/*Set second Camera*/
	mat4_identity(state.cam[1]);
	mat4_translate(state.cam[1], trans);
	mat4_rotate(state.cam[1], upy, 40.0f);

	/*Set third Camera*/
	mat4_identity(state.cam[2]);
	mat4_translate(state.cam[2], trans);
	mat4_rotate(state.cam[2], upx, 40.0f);
	mat4_rotate(state.cam[2], upy, 90.0f);

	/*Set lights and material*/
	state.light_act = 3;
	state.has_anim = 0;
	gfxDisplayRect(0, 0, DISP_W, DISP_H);
	gfxLightActive(state.light_act);
	gfxLightSet(0, &light_white);
	gfxLightSet(1, &light_blue);
	gfxLightViewUpdate(state.cam[state.camera_mat]);
	gfxSet(GFX_DEPTH_TEST, TRUE);
	gfxSet(GFX_LIGHTING_MODE, GFX_LIGHT_PHONG);

	/*Initialize the meshes that we can display*/
	printf("START MESH READ: res/mesh/bunny.obj...\n");
	gfxMeshLoad(state.mesh, "res/mesh/bunny.obj");
	printf("MESH READ DONE.\n");
	printf("START MESH READ: res/mesh/statue.obj...\n");
	gfxMeshLoad(state.mesh+1, "res/mesh/statue.obj");
	printf("MESH READ DONE.\n");
	state.mesh[0].mtrl = mtrl_set[state.mtrl_mode];
	state.mesh[1].mtrl = mtrl_set[state.mtrl_mode];
	g_mesh = state.mesh;

	state.texture[0] = gfxTexLoadBMP("res/textures/wood.bmp");
	printf("TEXTURE READ: res/textures/wood.bmp\n");
	state.texture[1] = gfxTexLoadBMP("res/textures/stone.bmp");
	printf("TEXTURE READ: res/textures/stone.bmp\n");
	g_tex = state.texture[0];
}
