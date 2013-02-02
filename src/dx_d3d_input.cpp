#include "dx_d3d_input.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

bool createD3DInputSystem(IInputSystem** out,HWND hwnd, HINSTANCE hInst, bool exclusive)
{
	if (*out == 0)
	{
		*out = new D3DInputSystem(hwnd,hInst,exclusive);
		return true;
	}
	return false;
}

bool D3DKeyboard::keyUp(unsigned int key)
{
	return (!(mKeys[key] & 0x80) && mKeys[key] != mOldKeys[key]);
}

bool D3DKeyboard::keyDown(unsigned int key)
{
	return (mKeys[key] & 0x80) != 0x00;
}

bool D3DKeyboard::capture()
{
	if (mDevice == 0)
	{
		return false;
	}

	memcpy(mOldKeys,mKeys,KEY_SIZE);

	if (FAILED(mDevice->GetDeviceState(KEY_SIZE,mKeys)))
	{
		if (FAILED(mDevice->Acquire()))
			return false;
		if (FAILED(mDevice->GetDeviceState(KEY_SIZE,mKeys)))
			return false;

		return true;
	}
	return true;
}

void D3DKeyboard::release()
{
	if (mDevice != 0)
	{
		mDevice->Unacquire();
		mDevice->Release();
		mDevice = 0;
	}
}

D3DKeyboard::D3DKeyboard(HWND hwnd,LPDIRECTINPUT8 input)
{
	if (input->CreateDevice(GUID_SysKeyboard,&mDevice,0) == DI_OK)
	{
		if (mDevice->SetDataFormat(&c_dfDIKeyboard) == DI_OK)
		{
			if (mDevice->SetCooperativeLevel(hwnd,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE) == DI_OK)
			{
				mDevice->Acquire();
				memset(mKeys,0,KEY_SIZE);
				memset(mOldKeys,0,KEY_SIZE);
			}
		}
	}
	//log("create keyboard device succeed");
}

D3DMouse::D3DMouse(HWND hwnd,LPDIRECTINPUT8 input,bool exclusive)
{
	mX = mY = mZ = 0;
	mHwnd = hwnd;
	if (input->CreateDevice(GUID_SysMouse,&mDevice,0) == DI_OK)
	{
		if (mDevice->SetDataFormat(&c_dfDIMouse) == DI_OK)
		{
			DWORD flag;
			if (exclusive)
			{
				flag = DISCL_FOREGROUND | DISCL_EXCLUSIVE | DISCL_NOWINKEY;
			}
			else
			{
				flag = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
			}
			if (mDevice->SetCooperativeLevel(hwnd,flag) == DI_OK)
			{
				mDevice->Acquire();
			}
		}
	}
	//log("create mouse device succeed");
}

bool D3DMouse::buttonDown(unsigned int btn)
{
	return (mState.rgbButtons[btn] & 0x80) != 0x00;
}

bool D3DMouse::buttonUp(unsigned int btn)
{
	return !(mState.rgbButtons[btn] & 0x80) && mState.rgbButtons[btn] != mOldState.rgbButtons[btn];
}

bool D3DMouse::capture()
{
	if (mDevice == 0)
		return false;

	memcpy(&mOldState,&mState,sizeof(mState));
	if (FAILED(mDevice->GetDeviceState(sizeof(mState),&mState)))
	{
		if (FAILED(mDevice->Acquire()))
			return false;
		if (FAILED(mDevice->GetDeviceState(sizeof(mState),&mState)))
			return false;
	}

	mX += mState.lX;
	mY += mState.lY;
	mZ += mState.lZ;
	return true;
}

void D3DMouse::release()
{
	if (mDevice != 0)
	{
		mDevice->Unacquire();
		mDevice->Release();
		mDevice = 0;
	}
}

D3DInputSystem::D3DInputSystem(HWND hwnd, HINSTANCE hInst, bool exclusive)
{
	mKeyborad = 0;
	mMouse = 0;
	mInputSys = 0;

	if (DirectInput8Create(hInst,DIRECTINPUT_VERSION,IID_IDirectInput8,(LPVOID*)&mInputSys,0) == DI_OK)
	{
		//log("create dinput succeed");
		mKeyborad = new D3DKeyboard(hwnd,mInputSys);
		mMouse = new D3DMouse(hwnd,mInputSys,exclusive);
	}
	else
	{
		//log("create dinput failed");
	}
}

bool D3DInputSystem::initialize()
{
	return mInputSys != 0 &&  mKeyborad != 0 && mMouse != 0;
}

bool D3DInputSystem::capture()
{
	if (mInputSys == 0)
	{
		return false;
	}

	bool bk,bm;
	if (mKeyborad != 0)
	{
		bk = mKeyborad->capture();
	}
	if (mMouse != 0)
	{
		bm = mMouse->capture();
	}
	return bk && bm;
}

bool D3DInputSystem::keyDown(unsigned int key)
{
	if (mKeyborad != 0)
	{
		return mKeyborad->keyDown(key);
	}
	return false;
}

bool D3DInputSystem::keyUp(unsigned int key)
{
	if (mMouse != 0)
	{
		return mKeyborad->keyUp(key);
	}
	return false;
}

bool D3DInputSystem::mouseButtonDown(unsigned int key)
{
	if (mMouse != 0)
	{
		return mMouse->buttonDown(key);
	}
	return false;
}

bool D3DInputSystem::mouseButtonUp(unsigned int key)
{
	if (mMouse != 0)
	{
		return mMouse->buttonUp(key);
	}
	return false;
}

Point D3DInputSystem::getMouseClientPosition()
{
	Point p = {0,0};
	if (mMouse != 0)
	{
		return mMouse->getClientPosition();
	}
	return p;
}

long D3DInputSystem::getWheelPosition()
{
	if (mMouse != 0)
	{
		return mMouse->getZ();
	}
	return 0;
}

void D3DInputSystem::release()
{
	if(mKeyborad != 0)
	{
		mKeyborad->release();
		delete mKeyborad;
	}
	if (mMouse != 0)
	{
		mMouse->release();
		delete mMouse;
	}

	if (mInputSys != 0)
		mInputSys->Release();
}

long D3DInputSystem::accX()
{
	if (mMouse != 0)
	{
		return mMouse->accX();
	}
	return 0;
}

long D3DInputSystem::accY()
{
	if (mMouse != 0)
	{
		return mMouse->accY();
	}
	return 0;
}

long D3DInputSystem::accZ()
{
	if (mMouse != 0)
	{
		return mMouse->accZ();
	}
	return 0;
}