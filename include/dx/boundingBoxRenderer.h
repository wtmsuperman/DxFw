#ifndef __D3D_BOUNDINGBOX_RENDERER__
#define __D3D_BOUNDINGBOX_RENDERER__

#include "mathlib.h"
#include <d3d9.h>
#include <d3dx9.h>

class IBoundingBoxRenderer
{
public:
	virtual void draw(const AABB3& boundingbox,unsigned long color = 0x88ff0000) = 0;
	virtual void release() = 0;
};

class D3DBoundingBoxRenderer : public IBoundingBoxRenderer
{
public:
	struct Vertex
	{
		float x,y,z;
		D3DCOLOR color;

		Vertex(float _x,float _y,float _z,long _color)
			:x(_x),y(_y),z(_z),color(_color)
		{}
		Vertex(const Vector3& v,long _color)
			:x(v.x),y(v.y),z(v.z),color(_color)
		{}
	};

	static const long FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

	D3DBoundingBoxRenderer(IDirect3DDevice9* device)
	{
		mMesh = 0;
		mDevice = device;
		D3DXMatrixIdentity(&mWorldIdentity);
	}

	~D3DBoundingBoxRenderer()
	{
		release();
	}

	virtual void draw(const AABB3& boundingbox,unsigned long color = 0x88ff0000);
	virtual void release()
	{
		if (mMesh != 0)
			mMesh->Release();
	}

private:
	IDirect3DDevice9* mDevice;
	ID3DXMesh* mMesh;
	D3DXMATRIX mWorldIdentity;
};

extern "C"
{
	void createD3DBoundingBoxRenderer(IBoundingBoxRenderer** out,IDirect3DDevice9* device);
};

#endif