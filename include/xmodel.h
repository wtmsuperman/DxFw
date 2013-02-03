#ifndef __XModel__
#define __XModel__

#include "dx/dx_resource.h"
#include "dx/dx_renderer.h"

class XModel : public IRenderable
{
public:
	


	virtual void preRender(DxRenderer* renderer)
	{}

	virtual void onRender(DxRenderer* renderer);

	virtual void postRender(DxRenderer* renderer)
	{}

	
	DxModel*	mXmodel;
};


#endif