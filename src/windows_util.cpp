#include "windows_util.h"

LRESULT WINAPI MsgProc(HWND hwnd,UINT msg,WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
#ifdef _DEBUG
	case WM_KEYUP:
		if (wp == VK_ESCAPE)
			::PostQuitMessage(0);
		break;
#endif
	}
	return DefWindowProc(hwnd,msg,wp,lp);
}

void messagePump(MSG* msg)
{
	if (PeekMessage(msg,0,0,0,PM_REMOVE))
	{
		TranslateMessage(msg);
		DispatchMessage(msg);
	}
}

bool initWindow(int width,int height,bool isFullScreen,const char* title,HINSTANCE hist,WinInfo* out,const char* className)
{

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc,
		0L, 0L, GetModuleHandle(NULL), NULL, NULL,
		NULL, NULL, className,NULL };
	RegisterClassEx(&wc);
	HWND hwnd;
	if (isFullScreen)
	{
		hwnd = CreateWindowEx(0,className,title,WS_POPUP | WS_VISIBLE
			,0,0,width,height,0,0,hist,0);
	}
	else
	{
		hwnd = CreateWindowEx(0, className, title,
			WS_OVERLAPPED| WS_CAPTION | WS_VISIBLE, 0,
			0, width, height,
			NULL, NULL, hist, NULL);
		//因为createwindow指定的长宽是window的长宽，而我们所关心的是客户区的长宽，因此需要进行调整
		RECT wr;
		RECT cr;
		GetWindowRect(hwnd,&wr);
		GetClientRect(hwnd,&cr);
		wr.right += width - cr.right+cr.left;
		wr.bottom += height - cr.bottom+cr.top;
		MoveWindow(hwnd,wr.left,wr.top,wr.right-wr.left,wr.bottom-wr.top,true);
	}
	if (hwnd)
	{
		ShowWindow(hwnd,SW_SHOW);
		UpdateWindow(hwnd);
	}
	else
	{
		//postErrorMsg("Create Window Failed");
		return false;
	}
	out->hist = hist;
	out->hwnd = hwnd;
	out->height = height;
	out->width = width;
	out->isFullScreen = isFullScreen;
	strcpy(out->title,title);
	strcpy(out->className,className);
	return true;
	//////////////////////////////////////////////////////////////////////////////////////////////////
}
