#ifndef __SCENE_NODE__
#define __SCENE_NODE__

#include "dx_renderer.h"
#include "node.h"

class SceneNode : public Node , public IRenderable
{
public:
	SceneNode(const char* name)
	{
		mName = new char[strlen(name)+1];
		strcpy(mName,name);
	}
	virtual ~SceneNode()
	{
		safe_delete(mName);
	}

	//override to set the render state if necessary
	virtual void preRender(DxRenderer* renderer)
	{
		if (mRenderObject)
			mRenderObject->preRender(renderer);
	}
	//override to render object
	virtual void onRender(DxRenderer* renderer)
	{
		if (mRenderObject)
		{
			Matrix4x4 m;
			generateLocalToParentMatrix(&m);
			renderer->setWorldTransform(m);

			mRenderObject->onRender(renderer);
		}
	}

	//override to reset the render state if necessary
	virtual void postRender(DxRenderer* renderer)
	{
		if (mRenderObject)
		{
			mRenderObject->postRender(renderer);
		}
	}

	void attach(IRenderable* obj)
	{
		this->mRenderObject = obj;
	}

	IRenderable* detach()
	{
		IRenderable* obj = mRenderObject;
		mRenderObject = 0;
		return obj;
	}

	char* getName() const {return mName;}

protected:
	char*			mName;
	IRenderable*	mRenderObject;
};

#endif