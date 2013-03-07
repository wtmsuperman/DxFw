#ifndef __DX_DEFINE__
#define __DX_DEFINE__

#define DIRECTINPUT_VERSION 0x0800

#include <d3dx9.h>
#include <d3d9.h>
#include <mathlib.h>
#include <dinput.h>
#include <assert.h>

#define MAX_TEXTURE 1

typedef unsigned long	DxColor32;

typedef D3DXCOLOR		DxColor;
typedef D3DCOLORVALUE	DxColorValue;


//call the Release() member function

template <class T>
void safe_Release(T& obj)
{
	if (obj != 0)
	{
		obj->Release();
		obj = 0;
	}
}

//call the release() member function
template <class T>
inline void safe_release(T& obj)
{
	if (obj != 0)
	{
		obj->release();
		obj = 0;
	}
}

template <class T>
inline void safe_delete(T& obj)
{
	if (obj != 0)
	{
		delete obj;
		obj = 0;
	}
}

template <class T>
inline void safe_deleteArray(T& obj)
{
	if (obj != 0)
	{
		delete[] obj;
		obj = 0;
	}
}

struct ltstr  
{  
  bool operator()(const char* s1, const char* s2) const  
  {  
    return strcmp(s1, s2) < 0;  
  }  
};

inline DWORD FtoDW(float f)
{
	return *((DWORD*)(&f));
}

#endif