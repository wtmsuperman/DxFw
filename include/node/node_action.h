#ifndef __ACT_NODE_ACTION__
#define __ACT_NODE_ACTION__

#include "node.h"
#include "action.h"
#include "route/path.h"

class TranslateToAction : public IAction
{
public:
	Node*					node;
	Path*					path;		
	Node::TransformSpace	transformSpace;
	TranslateToAction(Node* node,const Vector3& position,float duration,Node::TransformSpace ts)
	{
		this->node = node;
		transformSpace = ts;
		switch (ts)
		{
		case Node::TS_LOCAL:
			path = new LinearPathRel(position,duration);
			break;
		case Node::TS_PARENT:
			path = new LinearPath(node->getPosition(),position,duration);
			break;
		}
	}

	virtual ~TranslateToAction()
	{
		safe_delete(path);
	}

	virtual bool act(float delta)
	{
		Vector3 v;
		bool isFinish;
		switch (transformSpace)
		{
		case Node::TS_LOCAL:
			isFinish = path->calcPosition(delta,&v);
			node->translate(v,Node::TS_LOCAL);
			break;
		case Node::TS_PARENT:
			isFinish = path->calcPosition(delta,&v);
			node->setPosition(v);
			break;
		}
		return isFinish;
	}
};

#endif