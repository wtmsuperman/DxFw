#ifndef __NODE_I_ATTACHABLE__
#define __NODE_I_ATTACHABLE__

#include <dx/dx_renderer.h>
#include "scene_node.h"

// A class which implements this interface
// could be attached by a node or scene node(or subclass of node)
class AttachableObject : public IRenderable
{
public:
	AttachableObject()
		:mParent(0)
	{
	}

	virtual ~AttachableObject()
	{
		if (mParent)
		{
			((SceneNode*)mParent)->detachObject(this);
		}
	}

	virtual void notifyAttached(SceneNode* node)
	{
		mParent = node;
	}

	virtual void update(float delta) = 0;

	SceneNode* getParent()
	{
		return mParent;
	}

	virtual void preRender(DxRenderer* renderer){}
	virtual void onRender(DxRenderer* renderer) {}
	virtual void postRender(DxRenderer* renderer) {}

protected:
	SceneNode*	mParent;
};

#endif