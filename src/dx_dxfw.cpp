#include "dx_dxfw.h"
#include "dx_renderer.h"
#include "dx_resource_manager.h"
#include "dx_d3d_input.h"
#include "dx_input_interface.h"

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"winmm.lib")

#pragma comment(lib,"../Debug/mathlib.lib")

DxFw::DxFw()
{
	mDevice = 0;
	mDirect3D = 0;
	mRenderer = 0;
	mResourceMgr = 0;
	mInputSys = 0;
	ZeroMemory(&mDxParam,sizeof(DxParam));
}

DxFw::~DxFw()
{
	release();
}

bool DxFw::initInput(HWND hwnd,HINSTANCE hist,bool exclusive)
{
	return createD3DInputSystem(&mInputSys,hwnd,hist,exclusive);
}

bool DxFw::initDx(const DxParam& param)
{
	mDxParam = param;

	mDirect3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(mDirect3D == 0)
	{
		//postErrorMsg("Create D3D Failed");
		return false;
	}

	D3DDISPLAYMODE d3dMode;
	D3DCAPS9 d3dCaps;
	D3DPRESENT_PARAMETERS d3dParm;

	ZeroMemory(&d3dParm,sizeof(d3dParm));

	HRESULT hr;
	hr = mDirect3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&d3dMode);
	if (FAILED(hr))
	{
		//postErrorMsg("Get Adapter failed");
		return false;
	}

	hr = mDirect3D->GetDeviceCaps(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,&d3dCaps);
	if (FAILED(hr))
	{
		//postErrorMsg("Get Device caps failed");
		return false;
	}

	DWORD processing = 0;
	if (d3dCaps.VertexProcessingCaps != 0)
	{
		//使用了纯设备，这里要注意一下，可能会有性能上的问题
		processing = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
	}
	else
	{
		processing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	if(param.isFullScreen)
	{
		
		d3dParm.FullScreen_RefreshRateInHz = d3dMode.RefreshRate; 
		d3dParm.PresentationInterval = D3DPRESENT_INTERVAL_ONE; //垂直同步
		d3dParm.BackBufferCount = 0;
	}
	else
	{
		d3dParm.FullScreen_RefreshRateInHz = 0; //窗口模式下，必须为0
		d3dParm.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		d3dParm.BackBufferCount = 1;
	}

	d3dParm.BackBufferWidth = param.width;
	d3dParm.BackBufferHeight = param.height;
	d3dParm.Windowed = !param.isFullScreen;
	d3dParm.hDeviceWindow = param.hwnd;
	d3dParm.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dParm.BackBufferFormat = d3dMode.Format;
	d3dParm.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dParm.EnableAutoDepthStencil = true;
	d3dParm.Flags = 0;
	d3dParm.MultiSampleType = D3DMULTISAMPLE_NONE; //抗锯齿
	d3dParm.MultiSampleQuality = 0;

	hr = mDirect3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,param.hwnd,processing,&d3dParm,&mDevice);
	if (FAILED(hr))
	{
		return false;
	}

	mRenderer = new DxRenderer(mDevice);
	mResourceMgr = new DxResourceManager(mDevice);
	if (mRenderer == 0 || mResourceMgr == 0)
	{
		return false;
	}

	return true;
}

void DxFw::release()
{
	safe_delete(mRenderer);
	safe_delete(mResourceMgr);
	safe_Release(mDevice);
	safe_Release(mDirect3D);
	safe_release(mInputSys);
}