#ifndef __M_DX_RENDERER__
#define __M_DX_RENDERER__

#include "dx_defines.h"
#include "dx_resource.h"
#include <vector>

class DxRenderer;

struct IRenderable
{
	virtual ~IRenderable(){}
	//override to set the render state if necessary
	virtual void preRender(DxRenderer* renderer) = 0;
	//override to render object
	virtual void onRender(DxRenderer* renderer) = 0;
	//override to reset the render state if necessary
	virtual void postRender(DxRenderer* renderer) = 0;
};

class DxRenderer
{
public:
	explicit			DxRenderer(IDirect3DDevice9* device);

	void				clear(bool target,bool depth,bool stencil,DWORD color);

	inline void			setViewMatrix(const Matrix4x4& m);
	inline void			setWorldTransform(const Matrix4x4& m);
	inline void			beginScene();
	inline void			endScene();
	inline void			present();
	inline void			setFillMode(D3DFILLMODE mode);
	inline void			setRenderState(D3DRENDERSTATETYPE state,DWORD value);

	inline void			enableLight(bool enable);

	inline void			setLight(DWORD index,void* data);
	inline void			enableLight(DWORD index,bool enable);

	inline void			enableTransparent();
	inline void			disableTransparent();

	void				applyTexture(DWORD stage, const DxTexture* texture);
	void				setMaterial(const DxMaterial* mtrl);
	void				render(const DxBuffer* buffer);

	void				setAsPerspectiveProjection(float fov,float aspect,float n,float f);
	void				setDefaultSetSamplerState();
	

	void				render(IRenderable** renderList,size_t size);
	void				render(IRenderable* renderObject);
	
	IDirect3DDevice9*	getDevice() const {return mDevice;}
	
private:
	Matrix4x4			mProj;
	IDirect3DDevice9*	mDevice;
};



void DxRenderer::setViewMatrix(const Matrix4x4& m)
{
	mDevice->SetTransform(D3DTS_VIEW,&D3DXMATRIX(m.m));
}

void DxRenderer::setWorldTransform(const Matrix4x4& w)
{
	mDevice->SetTransform(D3DTS_WORLD,&D3DXMATRIX(w.m));
}

void DxRenderer::beginScene()
{
	mDevice->BeginScene();
}

void DxRenderer::endScene()
{
	mDevice->EndScene();
}

void DxRenderer::present()
{
	mDevice->Present(0,0,0,0);
}

void DxRenderer::setFillMode(D3DFILLMODE mode)
{
	mDevice->SetRenderState(D3DRS_FILLMODE,mode);
}


void DxRenderer::setRenderState(D3DRENDERSTATETYPE state,DWORD value)
{
	mDevice->SetRenderState(state,value);
}

void DxRenderer::enableLight(bool enable)
{
	mDevice->SetRenderState(D3DRS_LIGHTING,enable);
}

void DxRenderer::setLight(DWORD index,void* data)
{
	if (data != 0)
	{
		mDevice->SetLight(index,(D3DLIGHT9*)data);
		mDevice->LightEnable(index,true);
	}
}

void DxRenderer::enableLight(DWORD index,bool enable)
{
	mDevice->LightEnable(index,enable);
}

void DxRenderer::enableTransparent()
{
	mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true);
	mDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	mDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVDESTALPHA);
}

void DxRenderer::disableTransparent()
{
	mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,false);
}

#endif