#include "xmodel.h"

void XModel::onRender(DxRenderer* renderer)
{
	unsigned int faceNum = mXmodel->faceNum;
	for (unsigned int i=0; i < faceNum; ++i)
	{
		renderer->setMaterial(mXmodel->mtrls[i]);
		mXmodel->mesh->meshData->DrawSubset(i);
	}
}