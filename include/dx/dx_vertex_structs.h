#ifndef __DX_VERTEX_STRUCT__
#define __DX_VERTEX_STRUCT__

#include "dx_defines.h"
#include <mathlib.h>

typedef struct stColorVertex
{
	float x,y,z;
	DWORD color;

	stColorVertex(const Vector3& v,DWORD Color)
		:x(v.x),y(v.y),z(v.z),color(Color)
	{}

	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
}
ColorVertex;

typedef struct stVertex
{
	float x,y,z;

	stVertex(const Vector3& v)
		:x(v.x),y(v.y),z(v.z)
	{}

	static const DWORD FVF = D3DFVF_XYZ;
}
Vertex;

typedef struct stTexVertex
{
	float x,y,z;
	float u,v;

	stTexVertex(const Vector3& v,float _u,float _v)
		:x(v.x),y(v.y),z(v.z),u(_u),v(_v)
	{}

	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_TEX1;
}
TexVertex;

#endif;