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
		mName = 0;
	}

	AttachableObject(const char* name)
	{
		mName = new char[strlen(name) +1];
		strcpy(mName,name);
	}

	~AttachableObject()
	{
		safe_deleteArray(mName);
	}

	virtual void notifyAttached(Node* node)
	{
		mParent = node;
	}

	virtual void update(float delta) = 0;

	Node* getParent()
	{
		return mParent;
	}

protected:
	Node*	mParent;
	char*	mName;
};

#endif