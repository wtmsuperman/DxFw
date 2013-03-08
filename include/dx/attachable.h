#ifndef __NODE_I_ATTACHABLE__
#define __NODE_I_ATTACHABLE__

#include <dx/dx_renderer.h>

class IAttachable : public IRenderable
{
	virtual void notifyAttached(Node* node);
};

#endif