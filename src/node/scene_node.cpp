#include "node/scene_node.h"

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
		((SceneNode*)(iter->second))->preRender(renderer);
		((SceneNode*)(iter->second))->onRender(renderer);
		((SceneNode*)(iter->second))->postRender(renderer);
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
	assert(getObject(obj->getName())==0 && "already exist node");
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

AttachableObject* SceneNode::detachObject(const char* name)
{
	AttachedObjectListIter end = mAttachedObjects.end();
	for (AttachedObjectListIter iter=mAttachedObjects.begin(); iter!=end; ++iter)
	{
		if (strcmp(name,(*iter)->getName())== 0)
		{
			AttachableObject* obj = *iter;
			obj->notifyAttached(0);
			mAttachedObjects.erase(iter);
			return obj;
		}
	}
	return 0;
}

AttachableObject* SceneNode::getObject(const char* name)
{
	AttachedObjectListIter end = mAttachedObjects.end();
	for (AttachedObjectListIter iter=mAttachedObjects.begin(); iter!=end; ++iter)
	{
		if (strcmp(name,(*iter)->getName())== 0)
		{
			return *iter;
		}
	}
	return 0;
}
