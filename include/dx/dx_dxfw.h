#ifndef __DX_DXFW__
#define __DX_DXFW__

#include "dx_defines.h"
#include <Windows.h>
#include "dx_input_interface.h"


class DxRenderer;
class DxResourceManager;

inline float getTimeSinceLastFrame()
{
	static long lastTime = GetTickCount();
	long currentTime = GetTickCount();
	long timeSinceLastFrame = currentTime - lastTime;
	lastTime = currentTime;
	return timeSinceLastFrame * 0.001f;
}

typedef struct stDxParam
{
	bool	isFullScreen;
	int		width;
	int		height;
	HWND	hwnd;
}DxParam;

class DxFw
{
public:
	DxFw();
	~DxFw();

	bool				initDx(const DxParam& param);
	bool				initInput(HWND hwnd,HINSTANCE hist,bool exclusive);
	void				release();

	DxRenderer*			getRenderer() const {return mRenderer;}
	DxResourceManager*	getResourceManager() const {return mResourceMgr;}
	IDirect3DDevice9*	getDevice() const {return mDevice;}
	IInputSystem*		getInputSystem() const {return mInputSys;}
	const DxParam*		getDxParam() const {return &mDxParam;}
private:
	DxRenderer*			mRenderer;
	DxResourceManager*	mResourceMgr;
	IInputSystem*		mInputSys;
	IDirect3D9*			mDirect3D;
	IDirect3DDevice9*	mDevice;
	DxParam				mDxParam;
};

#endif