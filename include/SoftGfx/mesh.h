/*
 * SoftGfx - 1.0 - public domain
 * mesh.h: Mesh related types and functions
 */

#ifndef __MESH_H__
#define __MESH_H__


#include <SoftGfx/types.h>
#include <SoftGfx/light.h>


//structure for mesh
typedef struct Mesh_t {
	Vert*		vrtx;		//pointer to Verts
	u32*		indx;		//pointer to indeces
	u32			vrtx_count;	//number of Verts
	u32			indx_count;	//number of indices
	/* Mesh info for rendering */
	Material	mtrl;		//Mesh material
	mat4 		model;		//mesh matrix
} Mesh;

void gfxMeshLoad(Mesh *msh, const char *filename);
void gfxMeshFree(Mesh *msh);

#endif /*__MESH_H__*/
