#ifndef __DX_LOGGING__
#define __DX_LOGGING__

class GUILabel;

extern void loggingInit(GUILabel* l);
extern void logToScreen(const char* tag,const char* msg,...);

#endif