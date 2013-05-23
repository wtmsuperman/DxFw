#ifndef __SCENE_NODE_CONTAINER__
#define __SCENE_NODE_CONTAINER__

#include <map>
#include <list>
#include "dx/dx_renderer.h"
#include "node.h"

class AttachableObject;


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

	virtual ~SceneNode() {destroyAllChild();}

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

	void attachObject(AttachableObject* obj);

	void detachObject(AttachableObject* obj);

	void detachAll();

	virtual void update(float delta);

protected:
	typedef std::list<AttachableObject*>			AttachedObjectList;
	typedef AttachedObjectList::iterator			AttachedObjectListIter;

	AttachedObjectList								mAttachedObjects;
};

#endif