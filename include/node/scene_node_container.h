#ifndef __SCENE_NODE_CONTAINER__
#define __SCENE_NODE_CONTAINER__

#include <map>
#include "dx/dx_renderer.h"

class SceneNode;

class SceneNodeContainer : public IRenderable
{
public:
	SceneNodeContainer(){}
	~SceneNodeContainer() {destroyAll();}

	SceneNode*	createNode(const char* name);
	SceneNode*	removeNode(const char* name);
	void		destroyNode(const char* name);

	void		destroyAll();

	void		preRender(DxRenderer* render)
	{}

	void		onRender(DxRenderer* render);

	void		postRender(DxRenderer* render)
	{
		render->setWorldTransform(Matrix4x4::IDENTITY);
	}

protected:
	typedef std::map<const char*,SceneNode*,ltstr>	NodeType;
	typedef NodeType::iterator						NodeIter;
	typedef NodeType::const_iterator				ConstNodeIter;

	NodeType										mNodes;
	
};

#endif