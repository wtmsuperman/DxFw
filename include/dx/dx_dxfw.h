#ifndef __DX_DXFW__
#define __DX_DXFW__

#include "dx_defines.h"
#include <Windows.h>
#include "dx_input_interface.h"


class DxRenderer;
class DxResourceManager;
class DxParticleSystemManager;

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
	bool				initAll(const DxParam& parm,HWND hwnd,HINSTANCE hist,bool exclusive);
	bool				initDx(const DxParam& param);
	bool				initInput(HWND hwnd,HINSTANCE hist,bool exclusive);
	void				release();

	DxRenderer*			getRenderer() {return mRenderer;}
	DxResourceManager*	getResourceManager() {return mResourceMgr;}
	IDirect3DDevice9*	getDevice() {return mDevice;}
	IInputSystem*		getInputSystem() {return mInputSys;}
	const DxParam*		getDxParam() const {return &mDxParam;}

	DxRenderer*			getRenderer() const {return mRenderer;}
	DxResourceManager*	getResourceManager() const {return mResourceMgr;}
	IDirect3DDevice9*	getDevice() const {return mDevice;}
	IInputSystem*		getInputSystem() const {return mInputSys;}

	DxParticleSystemManager* getParticleSystemManager();

	void				saveScreenshot(const char* fileName);
	//save screenshot named by date
	void				saveScreenshot();

private:
	DxRenderer*					mRenderer;
	DxResourceManager*			mResourceMgr;
	DxParticleSystemManager*	mParticleSystemMgr;
	IInputSystem*				mInputSys;
	IDirect3D9*					mDirect3D;
	IDirect3DDevice9*			mDevice;
	DxParam						mDxParam;
};

#endif