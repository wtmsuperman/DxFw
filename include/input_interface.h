#ifndef __ME_INPUT_INTERFACE__
#define __ME_INPUT_INTERFACE__

#include "dx_defines.h"

struct Point
{
	long x,y;
};

class IInputSystem
{
public:
	virtual bool initialize() = 0;
	virtual void release() = 0;

	virtual bool capture() = 0;

	virtual bool keyUp(unsigned int key) = 0;
	virtual bool keyDown(unsigned int key) = 0;

	virtual bool mouseButtonDown(unsigned int btn) = 0;
	virtual bool mouseButtonUp(unsigned int btn) = 0;
	virtual Point getMouseClientPosition() = 0;
	virtual long getWheelPosition() = 0;
	
	virtual long accX() = 0;
	virtual long accY() = 0;
	virtual long accZ() = 0;
};

class IKeyboard
{
public:
	virtual void release() = 0;

	virtual bool keyUp(unsigned int key) = 0;
	virtual bool keyDown(unsigned int key) = 0;

	virtual bool capture() = 0;
};

class IMouse
{
public:
	virtual void release() = 0;

	virtual bool capture() = 0;

	virtual Point getClientPosition() = 0;

	virtual long getZ() = 0;

	virtual bool buttonDown(unsigned int key) = 0;
	virtual bool buttonUp(unsigned int key) = 0;

	virtual long accX() = 0;
	virtual long accY() = 0;
	virtual long accZ() = 0;
};

extern "C"
{
	bool createD3DInputSystem(IInputSystem** out,HWND hwnd, HINSTANCE hInst, bool exclusive);
}

#endif