#ifndef __XModel__
#define __XModel__

#include "dx/dx_resource.h"
#include "node/attachable.h"

class XModel : public AttachableObject
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