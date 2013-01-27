#include "dx_renderer.h"

DxRenderer::DxRenderer(IDirect3DDevice9* device)
	:mDevice(device)
{
	setDefaultSetSamplerState();
}

void DxRenderer::applyTexture(DWORD stage,const DxTexture* texture)
{
	if (texture == 0){
		mDevice->SetTexture(0,0);
	} else {	
		mDevice->SetTexture(stage,texture->data);
	}
}

void DxRenderer::clear(bool target,bool depth,bool stencil,DWORD color)
{
	DWORD flag = 0;
	if (target)
		flag |= D3DCLEAR_TARGET;
	if (depth)
		flag |= D3DCLEAR_ZBUFFER;
	if (stencil)
		flag |= D3DCLEAR_STENCIL;

	mDevice->Clear(0,0,flag,color,1.0f,0);
}

void DxRenderer::setMaterial(const DxMaterial* mtrl)
{
	if (mtrl == 0)
		return;
	else
	{
		mDevice->SetMaterial(&mtrl->mtrl);
		for (DWORD i=0; i < MAX_TEXTURE; ++i)
		{
			applyTexture(i,mtrl->textures[i]);
		}
	}
}

void DxRenderer::setAsPerspectiveProjection(float fov,float aspect,float n,float f)
{
	D3DXMATRIX m;
	D3DXMatrixPerspectiveFovLH(&m,fov,aspect,n,f);
	mDevice->SetTransform(D3DTS_PROJECTION,&m);
	mProj.set((float*)m);
}

void DxRenderer::setDefaultSetSamplerState()
{
	mDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	mDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	mDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

void DxRenderer::render(const DxBuffer* buffer)
{
	if (buffer == 0)
		return;

	unsigned base = 0;

	if (buffer->ib != 0)
	{
		mDevice->SetIndices(buffer->ib);
		base = buffer->indiceNum;
	}
	else
	{
		base = buffer->vertexNum;
	}

	mDevice->SetStreamSource(0,buffer->vb,0,buffer->stride);
	mDevice->SetFVF(buffer->fvf);

	unsigned int primitiveCount = 0;
	switch (buffer->type)
	{
	case D3DPT_POINTLIST:
		primitiveCount = base;
		break;
	case D3DPT_TRIANGLELIST:
		primitiveCount = base / 3;
		break;
	case D3DPT_TRIANGLEFAN:
		primitiveCount = base - 2;
		break;
	case D3DPT_TRIANGLESTRIP:
		primitiveCount = base - 2;
		break;
	case D3DPT_LINELIST:
		primitiveCount = base / 2;
		break;
	case D3DPT_LINESTRIP:
		primitiveCount = base - 1;
		break;
	}
	
	if (buffer->ib != 0)
	{
		mDevice->DrawIndexedPrimitive(buffer->type,0,0,buffer->vertexNum,0,primitiveCount);
	}
	else
	{
		mDevice->DrawPrimitive(buffer->type,0,primitiveCount);
	}
}

void DxRenderer::render(IRenderable** renderList,size_t size)
{
	for (size_t i=0; i < size; ++i)
	{
		renderList[i]->preRender(this);
		renderList[i]->onRender(this);
		renderList[i]->postRender(this);
	}
}

void DxRenderer::render(IRenderable* renderObject)
{
	renderObject->preRender(this);
	renderObject->onRender(this);
	renderObject->postRender(this);
}