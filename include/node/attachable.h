#ifndef __NODE_I_ATTACHABLE__
#define __NODE_I_ATTACHABLE__

#include <dx/dx_renderer.h>
#include "node.h"

// A class which implements this interface
// could be attached by a node or scene node(or subclass of node)
class AttachableObject : public IRenderable
{
public:
	AttachableObject()
		:mParent(0)
	{
	}

	virtual void notifyAttached(Node* node)
	{
		mParent = node;
	}

	Node* getParent()
	{
		return mParent;
	}

protected:
	Node*	mParent;
};

#endif