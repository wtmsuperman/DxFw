#include <node/node.h>
#include <string>

Node* Node::createChild(const char* name)
{
	assert(mNodes.find(name) == mNodes.end() && "already exists this node");
	Node* node = createChildImpl(name);
	node->parent = this;
	mNodes[name] = node;
	return node;
}

Node* Node::createChild()
{
	assert(mNodes.find(name) == mNodes.end() && "already exists this node");
	Node* node = createChildImpl();
	node->parent = this;
	mNodes["null"] = node;
	return node;
}

Node* Node::removeChild(const char* name)
{
	assert(mNodes.find(name) != mNodes.end() && "this node do not exists");
	NodeIter iter = mNodes.find(name);
	iter->second->parent = 0;
	mNodes.erase(iter);
	return iter->second;
}

Node* Node::getChild(const char* name)
{
	assert(mNodes.find(name) != mNodes.end() && "this node do not exists");
	return mNodes[name];
}

void Node::destroyChild(const char* name)
{
	assert(mNodes.find(name) != mNodes.end() && "this node do not exists");
	NodeIter iter = mNodes.find(name);
	mNodes.erase(iter);
	safe_delete(iter->second);
}

void Node::destroyAllChild()
{
	NodeIter end = mNodes.end();
	for (NodeIter iter = mNodes.begin(); iter != end; ++iter)
	{
		safe_delete(iter->second);
	}

	mNodes.clear();
}