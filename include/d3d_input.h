#ifndef __ME_D3D_INPUT__
#define __ME_D3D_INPUT__
#include "input_interface.h"


#define KEY_SIZE 256

class D3DInputSystem : public IInputSystem
{
public:
	D3DInputSystem(HWND hwnd, HINSTANCE hInst, bool exclusive);
	~D3DInputSystem()
	{
		release();
	}

	virtual bool initialize();
	virtual void release();

	virtual bool capture();

	virtual bool keyUp(unsigned int key);
	virtual bool keyDown(unsigned int key);

	virtual bool mouseButtonDown(unsigned int btn);
	virtual bool mouseButtonUp(unsigned int btn);
	virtual Point getMouseClientPosition();
	virtual long getWheelPosition();

	virtual long accX();
	virtual long accY();
	virtual long accZ();

private:
	IKeyboard* mKeyborad;
	IMouse* mMouse;
	LPDIRECTINPUT8 mInputSys;
};

class D3DKeyboard : public IKeyboard
{
public:
	D3DKeyboard(HWND hwnd,LPDIRECTINPUT8 input);

	virtual ~D3DKeyboard()
	{
		release();
	}
	virtual void release();

	virtual bool keyUp(unsigned int key);
	virtual bool keyDown(unsigned int key);

	virtual bool capture();
private:
	LPDIRECTINPUTDEVICE8 mDevice;
	char mKeys[KEY_SIZE];
	char mOldKeys[KEY_SIZE];
};

class D3DMouse : public IMouse
{
public:
	D3DMouse(HWND hwnd,LPDIRECTINPUT8 input,bool exclusive);

	virtual ~D3DMouse()
	{
		release();
	}

	virtual void release();
	virtual bool capture();

	Point getClientPosition()
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(mHwnd,&p);
		Point p0 = {p.x,p.y};
		return p0;
	}

	long getZ()
	{
		return mZ;
	}

	long accX()
	{
		return mState.lX;
	}

	long accY()
	{
		return mState.lY;
	}

	long accZ()
	{
		return mState.lZ;
	}

	bool buttonDown(unsigned int key);
	bool buttonUp(unsigned int key);

	bool mIsRestrict;
	int mWidth;
	int mHeight;

private:
	LPDIRECTINPUTDEVICE8 mDevice;
	DIMOUSESTATE mState;
	DIMOUSESTATE mOldState;
	HWND mHwnd;

	long mX,mY,mZ;
};

#endif
