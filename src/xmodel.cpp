#include "xmodel.h"
#include "node/node.h"

void XModel::onRender(DxRenderer* renderer)
{
	if (mParent)
	{
		Matrix4x4 m;
		mParent->generateLocalToParentMatrix(&m);
		renderer->setWorldTransform(m);
	}
	else
	{
		renderer->setWorldTransform(Matrix4x4::IDENTITY);
	}

	//renderer->setRenderState(D3DRS_LIGHTING,true);
	
	unsigned int faceNum = mXmodel->faceNum;
	for (unsigned int i=0; i < faceNum; ++i)
	{
		renderer->setMaterial(mXmodel->mtrls[i]);
		mXmodel->mesh->meshData->DrawSubset(i);
	}
}