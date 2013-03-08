#ifndef __SCENE_NODE__
#define __SCENE_NODE__

#include "dx/dx_renderer.h"
#include "node/node.h"
#include "node/attachable.h"
#include <list>

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
	}
	//override to render object
	virtual void onRender(DxRenderer* renderer)
	{
		AttachedObjectListIter end = mAttachedObjects.end();
		for (AttachedObjectListIter iter=mAttachedObjects.begin(); iter!=end; ++iter)
		{
			(*iter)->preRender(renderer);
			(*iter)->onRender(renderer);
			(*iter)->postRender(renderer);
		}
	}

	//override to reset the render state if necessary
	virtual void postRender(DxRenderer* renderer)
	{
	}

	void attach(AttachableObject* obj)
	{
		mAttachedObjects.push_back(obj);
		obj->notifyAttached(this);
	}

	void detach(AttachableObject* obj)
	{
		AttachedObjectListIter end = mAttachedObjects.end();
		for (AttachedObjectListIter iter=mAttachedObjects.begin(); iter!=end; ++iter)
		{
			if (*iter == obj)
			{
				mAttachedObjects.erase(iter);
				return;
			}
		}
	}

	char* getName() const {return mName;}

protected:
	char*											mName;
	
	typedef std::list<AttachableObject*>			AttachedObjectList;
	typedef AttachedObjectList::iterator			AttachedObjectListIter;

	AttachedObjectList								mAttachedObjects;
};

#endif