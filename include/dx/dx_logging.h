#ifndef __DX_LOGGING__
#define __DX_LOGGING__

class GUISystem;

extern void loggingInit(GUISystem* l);
extern void logToScreen(const char* tag,const char* msg,...);

#endif