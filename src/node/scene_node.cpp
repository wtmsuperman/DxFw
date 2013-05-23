#include "node/scene_node.h"
#include "node/attachable.h"


void SceneNode::onRender(DxRenderer* renderer)
{
	//render self
	AttachedObjectListIter end = mAttachedObjects.end();
	for (AttachedObjectListIter iter=mAttachedObjects.begin(); iter!=end; ++iter)
	{
		(*iter)->preRender(renderer);
		(*iter)->onRender(renderer);
		(*iter)->postRender(renderer);
	}

	//render child

	NodeIter node_end = mNodes.end();
	for (NodeIter iter = mNodes.begin(); iter != node_end; ++iter)
	{
		((SceneNode*)(iter->second))->onRender(renderer);
	}
}

void SceneNode::update(float delta)
{
	//render self
	AttachedObjectListIter end = mAttachedObjects.end();
	for (AttachedObjectListIter iter=mAttachedObjects.begin(); iter!=end; ++iter)
	{
		(*iter)->update(delta);
	}

	//render child

	NodeIter node_end = mNodes.end();
	for (NodeIter iter = mNodes.begin(); iter != node_end; ++iter)
	{
		((SceneNode*)(iter->second))->update(delta);
	}
}

void SceneNode::attachObject(AttachableObject* obj)
{		
	mAttachedObjects.push_back(obj);
	obj->notifyAttached(this);
}

void SceneNode::detachObject(AttachableObject* obj)
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

void SceneNode::detachAll()
{
	AttachedObjectListIter end = mAttachedObjects.end();
	for (AttachedObjectListIter iter=mAttachedObjects.begin(); iter!=end; ++iter)
	{
		(*iter)->notifyAttached(0);
	}

	mAttachedObjects.clear();
}