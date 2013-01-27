#ifndef __M_DX_RESOURCE__
#define __M_DX_RESOURCE__

#include "dx_defines.h"

typedef struct stDxBuffer
{
	char*						name;
	char*						groupName;
	LPDIRECT3DVERTEXBUFFER9		vb;
	LPDIRECT3DINDEXBUFFER9		ib;
	D3DPRIMITIVETYPE			type;
	unsigned int				vertexNum;
	unsigned int				indiceNum;
	unsigned int				stride;
	unsigned long				fvf;
}DxBuffer ;

typedef struct stDxTexture
{
	char*				name;
	char*				groupName;
	LPDIRECT3DTEXTURE9	data;
	unsigned int		width;
	unsigned int		height;
}DxTexture;

typedef struct stDxMaterial
{
	char*			name;
	char*			groupName;
	D3DMATERIAL9	mtrl;
	DxTexture*		textures[MAX_TEXTURE];
}DxMaterial;

typedef struct stDxMesh
{
	char*		name;
	char*		groupName;
	ID3DXMesh*	meshData;
}DxMesh;

typedef struct stDxModel
{
	char*			name;
	char*			groupName;
	DxMesh*			mesh;
	DxMaterial**	mtrls;
	unsigned int	faceNum;
}DxModel;

#endif