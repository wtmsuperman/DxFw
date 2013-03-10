#ifndef __SCENE_NODE_CONTAINER__
#define __SCENE_NODE_CONTAINER__

#include <map>
#include <list>
#include "dx/dx_renderer.h"
#include "attachable.h"


class SceneNode : public IRenderable,public Node
{
public:
	SceneNode(const char* name)
		:Node(name)
	{
		
	}

	SceneNode()
		:Node()
	{
	}

	~SceneNode() {destroyAllChild();}

	Node*		createChildImpl()
	{
		return new SceneNode;
	}
	Node*		createChildImpl(const char* name)
	{
		return new SceneNode(name);
	}

	void		preRender(DxRenderer* render)
	{}

	void		onRender(DxRenderer* render);

	void		postRender(DxRenderer* render)
	{}

	void attachObject(AttachableObject* obj)
	{
		mAttachedObjects.push_back(obj);
		obj->notifyAttached(this);
	}

	void detachObject(AttachableObject* obj)
	{
		AttachedObjectListIter end = mAttachedObjects.end();
		for (AttachedObjectListIter iter=mAttachedObjects.begin(); iter!=end; ++iter)
		{
			if (*iter == obj)
			{
				obj->notifyAttached(0);
				mAttachedObjects.erase(iter);
				return;
			}
		}
	}

	virtual void update(float delta);

protected:
	typedef std::list<AttachableObject*>			AttachedObjectList;
	typedef AttachedObjectList::iterator			AttachedObjectListIter;

	AttachedObjectList								mAttachedObjects;
};

#endif