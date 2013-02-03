#ifndef __WINDOWS_UTIL__
#define __WINDOWS_UTIL__

#include <Windows.h>

LRESULT WINAPI MsgProc(HWND hwnd,UINT msg,WPARAM wp, LPARAM lp);

void messagePump(MSG* msg);

struct WinInfo
{
	HINSTANCE hist;
	HWND hwnd;
	bool isFullScreen;
	int width;
	int height;
	char title[32];
	char className[32];
};

bool initWindow(int width,int height,bool isFullScreen,const char* title,HINSTANCE hist,WinInfo* out,const char* className = "dxfw");

#endif