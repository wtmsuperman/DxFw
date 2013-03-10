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

