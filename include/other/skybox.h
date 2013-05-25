#ifndef __SKY_BOX__
#define __SKY_BOX__

#include <dx/dx_defines.h>
#include <dx/dx_renderer.h>
#include <mathlib.h>

struct SkyVertex
{
	float x,y,z;
	float u,v;
	const static DWORD FVF = D3DFVF_XYZ | D3DFVF_TEX1;
};

class SkyBox
{
public:
	void init(IDirect3DDevice9* device,float size);
	void load(char *texfile[]);
	void render();
	void release();
	void translate(float x,float y,float z);
private:
	IDirect3DDevice9*			mDevice;
	float						mSize;
	IDirect3DVertexBuffer9*		mVertexBuffer;
	IDirect3DTexture9*			mTextures[6];
	Vector3						mPosition;
};

#endif