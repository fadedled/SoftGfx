/*
 * SoftGfx - 1.0 - public domain
 * mesh.c : Mesh related functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SoftGfx/mesh.h>
#include <SoftGfx/vm_math.h>
#include <math.h>



//#define HASH(i, j, k)		(((i ^ (j << 4)) >> 4) ^ (k << 8))
#define HASH(i, j, k)		((i << 10 ^ j << 10) | k)
//#define HASH(i, j, k)		(((((i + j)*(i + j + 1) >> 1)+ j + k)*(((i + j)*(i + j + 1) >> 1) + j + k + 1) >> 1) + k)


static const
Material STD_MTRL = {
	{0.2f, 0.2f, 0.2f},	//ambient
	{0.7f, 0.7f, 0.7f},	//diffuse
	{0.8f, 0.8f, 0.8f},	//specular
	32.0f	//shininess
};

struct WFVert {
	vec3 pos;
	vec3 norm;
	vec2 tex;
};

const char *FACE_STR[4] = {" %u", " %u/%u", " %u//%u", " %u/%u/%u"};

typedef struct RepItem_tag {
	u32 v0;
	u32 v1;
	u32 v2;
} RepItem;

struct Rep_arr {
	RepItem* data;
	u32		i;
	u32		size;
	u32		exists[0x80000];
}set_arr = {NULL, 0, 4096, {0}};

//==============================================================================
// HASH TABLE FOR REPEATED VERTICES
//==============================================================================
/* Bad implementation of set */
void
_setInit(void)
{
	set_arr.i = 0;
	set_arr.size = 1024;
	set_arr.data = calloc(set_arr.size << 1, sizeof(RepItem));
	memset(set_arr.exists, 0, sizeof(set_arr.exists));
}

void
_setQuit(void)
{
	free(set_arr.data);
}


void
_setDoubleSize(void)
{
	RepItem* tmp_data = set_arr.data;
	set_arr.data = calloc(set_arr.size << 1, sizeof(RepItem));
	memcpy(set_arr.data, tmp_data, set_arr.size * sizeof(RepItem));
	free(tmp_data);
	set_arr.size <<= 1;
}


u32
_setInsert(u32 i, u32 j, u32 k)
{
	i -= (i > 0 ? 1 : 0);
	j -= (j > 0 ? 1 : 0);
	k -= (k > 0 ? 1 : 0);
	if (set_arr.i == set_arr.size) {
		_setDoubleSize();
	}
	u32 h = HASH(i, j, k) & 0xFFFFFFu;
	//printf("%d\n", h >> 5);
	if ((set_arr.exists[h >> 5] >> (h & 0x1fu)) & 0x1) {
		//u32 indx = 0;
		//printf("WHAT\n");
		for (u32 indx = 0; indx < set_arr.i; ++indx) {
			if (set_arr.data[indx].v0 == i &&
				set_arr.data[indx].v1 == j &&
				set_arr.data[indx].v2 == k) {
					return indx;
			}
			//++indx;
		}
	}
	set_arr.data[set_arr.i].v0 = i;
	set_arr.data[set_arr.i].v1 = j;
	set_arr.data[set_arr.i].v2 = k;
	++set_arr.i;
	set_arr.exists[h >> 5] |= 1 << (h & 0x1fu);
	return set_arr.i - 1;
}


//==============================================================================

void
gfxMeshLoad(Mesh *msh, const char *filename)
{
	/*Open the file*/
	u32 nv = 0, nn = 0, nt = 0, nf = 0, max_v = 0;
	u32 has_n = 0, has_t = 0;
	char line[1024];
	struct WFVert *wfv;
	FILE *in = fopen(filename, "r");
	if (!in) {
 		printf("ERROR: The file %s was not found.", filename);
 		exit(0);
	}
	/*Read lines and count faces, positions, normals and textures*/
	while ((fgets(line, sizeof(line), in)) != NULL) {
		if (line[0] == 'v') {
			switch (line[1]) {
				case 'n': {++nn; has_n = 2;} break;
				case 't': {++nt; has_t = 1;} break;
				default: {++nv;} break;
			}
		} else if (line[0] == 'f') {
			nf += 8;	/*Aproximate number of indices*/
		}
 	}
	/*Find max number of vertices and create arrays for indices, Waveform vertx */
	if (nf == 0) {
		return;
	}
	max_v = (nv > nn ? nv : nn);
	max_v = (max_v > nt ? max_v : nt);
	u32* faces = (u32*) calloc(nf, sizeof(u32));
	wfv = (struct WFVert*) calloc(max_v * 4, sizeof(struct WFVert));

	/*Create the set for storing faces*/
	_setInit();
	nv = nf = nn = nt = 0;
	/*Reread line*/
	fseek(in, 0L, SEEK_SET);
	while((fgets(line, sizeof(line), in)) != NULL) {
		if (line[0] == 'v') {
			f32 x, y, z;
			switch (line[1]){
				/*Vertex pos*/
				case ' ': {
					sscanf(line, "v %f %f %f", &x, &y, &z);
					wfv[nv].pos[0] = x; wfv[nv].pos[1] = y; wfv[nv].pos[2] = z;
					nv++;
				} break;
				/*Vertex Normal*/
				case 'n': {
					sscanf(line, "vn %f %f %f", &x, &y, &z);
					wfv[nn].norm[0] = x; wfv[nn].norm[1] = y; wfv[nn].norm[2] = z;
					nn++;
				} break;
				/*Vertex Texture*/
				case 't':{
					sscanf(line, "vt %f %f", &x, &y);
					wfv[nt].tex[0] = x; wfv[nt].tex[1] = y;
					nt++;
				} break;
			}
		}
		/*Store indices by parsing the line*/
		else if (line[0] == 'f'){
			u32 v0 = 0, v1 = 0, v2 = 0;
			/*Format depends if there are normals or texutres*/
			switch (has_n + has_t) {
				case 0: {
					u32 vfirst, vlast;
					char *tok = strtok(line+2," /\n");
					v0 = atoi(tok);
					vfirst = _setInsert(v0, 0, 0);
					v0 = atoi(tok = strtok(NULL, " /\n"));
					vlast = _setInsert(v0, 0, 0);
					tok = strtok(NULL, " /\n");
					while (tok != NULL)
					{
						v0 = atoi(tok);
						faces[nf] = vfirst;
						faces[nf+1] = vlast;
						faces[nf+2] = vlast = _setInsert(v0, 0, 0);
						nf += 3;
						tok = strtok(NULL, " /\n");
					}
				} break;
				case 1: {
					u32 vfirst, vlast;
					char *tok = strtok(line+2," /\n");
					v0 = atoi(tok);
					v1 = atoi(tok = strtok(NULL, " /\n"));
					vfirst = _setInsert(v0, v1, 0);
					v0 = atoi(tok = strtok(NULL, " /\n"));
					v1 = atoi(tok = strtok(NULL, " /\n"));
					vlast = _setInsert(v0, v1, 0);
					tok = strtok(NULL, " /\n");
					while (tok != NULL)
					{
						v0 = atoi(tok);
						v1 = atoi(tok = strtok(NULL, " /\n"));
						faces[nf] = vfirst;
						faces[nf+1] = vlast;
						faces[nf+2] = vlast = _setInsert(v0, v1, 0);
						nf += 3;
						tok = strtok(NULL, " /\n");
					}
				} break;
				case 2: {
					u32 vfirst, vlast;
					char *tok = strtok(line+2," /\n");
					v0 = atoi(tok);
					v2 = atoi(tok = strtok(NULL, " /\n"));
					vfirst = _setInsert(v0, 0, v2);
					v0 = atoi(tok = strtok(NULL, " /\n"));
					v2 = atoi(tok = strtok(NULL, " /\n"));
					vlast = _setInsert(v0, 0, v2);
					tok = strtok(NULL, " /\n");
					while (tok != NULL) {
						v0 = atoi(tok);
						v2 = atoi(tok = strtok(NULL, " /\n"));
						faces[nf] = vfirst;
						faces[nf+1] = vlast;
						faces[nf+2] = vlast = _setInsert(v0, 0, v2);
						nf += 3;
						tok = strtok(NULL, " /\n");
					}
				} break;
				case 3: {
					u32 vfirst, vlast;
					char *tok = strtok(line+2," /\n");
					v0 = atoi(tok);
					v1 = atoi(tok = strtok(NULL, " /\n"));
					v2 = atoi(tok = strtok(NULL, " /\n"));
					vfirst = _setInsert(v0, v1, v2);
					v0 = atoi(tok = strtok(NULL, " /\n"));
					v1 = atoi(tok = strtok(NULL, " /\n"));
					v2 = atoi(tok = strtok(NULL, " /\n"));
					vlast = _setInsert(v0, v1, v2);
					tok = strtok(NULL, " /\n");
					while (tok != NULL)
					{
						v0 = atoi(tok);
						v1 = atoi(tok = strtok(NULL, " /\n"));
						v2 = atoi(tok = strtok(NULL, " /\n"));
						faces[nf] = vfirst;
						faces[nf+1] = vlast;
						faces[nf+2] = vlast = _setInsert(v0, v1, v2);
						nf += 3;
						tok = strtok(NULL, " /\n");
					}
				} break;
			}
		}
 	}

 	/*Store data in the mesh*/
	msh->indx_count = nf;
	msh->vrtx_count = set_arr.i;
	msh->vrtx = (Vert*) calloc(msh->vrtx_count, sizeof(Vert));
	msh->indx = (u32*) calloc(msh->indx_count, sizeof(u32));
	msh->mtrl = STD_MTRL;
	mat4_identity(msh->model);
	memcpy(msh->indx, faces, msh->indx_count * sizeof(u32));
	/*Create and store the new Vertices*/
	for (u32 i = 0; i < set_arr.i; ++i) {
		u32 pi = set_arr.data[i].v0;
		u32 ti = set_arr.data[i].v1;
		u32 ni = set_arr.data[i].v2;
		//printf("(pi, ti, ni) -> (%d, %d, %d)\n", pi, ti, ni);
		msh->vrtx[i].pos[0] = wfv[pi].pos[0];
		msh->vrtx[i].pos[1] = wfv[pi].pos[1];
		msh->vrtx[i].pos[2] = wfv[pi].pos[2];
		msh->vrtx[i].norm[0] = wfv[ni].norm[0];
		msh->vrtx[i].norm[1] = wfv[ni].norm[1];
		msh->vrtx[i].norm[2] = wfv[ni].norm[2];
		msh->vrtx[i].tex[0] = wfv[ti].tex[0];
		msh->vrtx[i].tex[1] = wfv[ti].tex[1];
		msh->vrtx[i].color[0] = 1.0f;
		msh->vrtx[i].color[1] = 1.0f;
		msh->vrtx[i].color[2] = 1.0f;
	}

	/*Calculate normals if there were none*/
	if (!has_n) {
		u32 count = msh->indx_count - (msh->indx_count % 3);
		u8 norm_count[2048];
		/* Add all face normals */
		for (u32 i = 0; i < count; i += 3) {
			vec3 vv, ww, tmp;
			vec3_sub(vv, msh->vrtx[msh->indx[i+1]].pos, msh->vrtx[msh->indx[i]].pos);
			vec3_sub(ww, msh->vrtx[msh->indx[i+2]].pos, msh->vrtx[msh->indx[i]].pos);
			vec3_cross(tmp, vv, ww);
			vec3_normalize(tmp);

			vec3_add(msh->vrtx[msh->indx[i]].norm, msh->vrtx[msh->indx[i]].norm, tmp);
			vec3_add(msh->vrtx[msh->indx[i+1]].norm, msh->vrtx[msh->indx[i+1]].norm, tmp);
			vec3_add(msh->vrtx[msh->indx[i+2]].norm, msh->vrtx[msh->indx[i+2]].norm, tmp);
			norm_count[msh->indx[i]]++;
			norm_count[msh->indx[i+1]]++;
			norm_count[msh->indx[i+2]]++;
		}
		/* Get average normal for each vertex */
		for (u32 i = 0; i < msh->vrtx_count; ++i) {
			if (norm_count[i]) {
				msh->vrtx[i].norm[0] /= (f32) norm_count[i];
				msh->vrtx[i].norm[1] /= (f32) norm_count[i];
				msh->vrtx[i].norm[2] /= (f32) norm_count[i];
				vec3_normalize(msh->vrtx[i].norm);
			}
		}
	}
	/*Calculate spherical texture mapping given the vertex normals*/
	if (!has_t) {
		for (u32 i = 0; i < msh->vrtx_count; ++i) {
			msh->vrtx[i].tex[0] = (asinf(msh->vrtx[i].norm[0]) / PI) + 0.5f;
			msh->vrtx[i].tex[1] = (asinf(msh->vrtx[i].norm[1]) / PI) + 0.5f;
		}
	}

	/*Free used dinamic data*/
	_setQuit();
	free(wfv);
	free(faces);
	fclose(in);
}


/*Frees the internal mesh arrays*/
void
gfxMeshFree(Mesh *msh)
{
	free(msh->vrtx);
	free(msh->indx);
}

