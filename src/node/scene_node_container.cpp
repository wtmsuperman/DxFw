#include "node/scene_node_container.h"
#include "node/scene_node.h"

void SceneNodeContainer::onRender(DxRenderer* renderer)
{
	renderer->beginScene();
	NodeIter end = mNodes.end();
	for (NodeIter iter = mNodes.begin(); iter != end; ++iter)
	{
		iter->second->onRender(renderer);
	}
	renderer->endScene();
}

SceneNode* SceneNodeContainer::createNode(const char* name)
{
	assert(mNodes.find(name) == mNodes.end() && "already exists this node");
	SceneNode* node = new SceneNode(name);;
	mNodes[name] = node;
	return node;
}

SceneNode* SceneNodeContainer::removeNode(const char* name)
{
	assert(mNodes.find(name) != mNodes.end() && "this node do not exists");
	NodeIter iter = mNodes.find(name);
	mNodes.erase(iter);
	return iter->second;
}

void SceneNodeContainer::destroyNode(const char* name)
{
	assert(mNodes.find(name) != mNodes.end() && "this node do not exists");
	NodeIter iter = mNodes.find(name);
	mNodes.erase(iter);
	safe_delete(iter->second);
}

void SceneNodeContainer::destroyAll()
{
	NodeIter end = mNodes.end();
	for (NodeIter iter = mNodes.begin(); iter != end; ++iter)
	{
		safe_delete(iter->second);
	}

	mNodes.clear();
}

