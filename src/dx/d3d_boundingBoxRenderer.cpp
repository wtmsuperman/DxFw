#include "dx/boundingBoxRenderer.h"

void D3DBoundingBoxRenderer::draw(const AABB3& boundingbox,unsigned long color)
{
	if (mMesh == 0) 
	{
		D3DXCreateMeshFVF(12,8,D3DXMESH_IB_MANAGED | D3DXMESH_VB_DYNAMIC | D3DXMESH_VB_WRITEONLY,FVF,mDevice,&mMesh);
		if (mMesh == 0)
		{
			MessageBox(0,"create boudingbox mesh error","",MB_OK);
			return;
		}

		///////////////////////////////////////////////////////////////
		WORD* i = 0;
		mMesh->LockIndexBuffer(0,(void**)&i);
		if (i == 0)
		{
			MessageBox(0,"lock boudingbox mesh index buffer error","",MB_OK);
			return;
		}
		i[0] = 0,i[1] = 1,i[2] = 2,  i[3] = 1,i[4] = 3,i[5] = 2;
		i[6] = 1,i[7] = 5,i[8] = 3,  i[9] = 5,i[10] = 7,i[11] = 3;
		i[12] = 2;i[13] = 3,i[14] = 6,  i[15] = 3,i[16] = 7,i[17] = 6;
		i[18] = 4,i[19] = 5,i[20] = 6,  i[21] = 5,i[22] = 7,i[23] = 6;
		i[24] = 0,i[25] = 1,i[26] = 5,  i[27] = 1,i[28] = 5,i[29] = 4;
		i[30] = 0,i[31] = 4,i[32] = 2,  i[33] = 4,i[34] = 6,i[35] = 2;
		mMesh->UnlockIndexBuffer();
		///////////////////////////////////////////////////////////////

		DWORD* attribute = 0;
		mMesh->LockAttributeBuffer(0,&attribute);
		if (attribute == 0)
		{
			MessageBox(0,"lock boudingbox mesh attribute buffer error","",MB_OK);
			return;
		}
		for (int j=0; j<12; ++j)
			attribute[j] = 0;
		mMesh->UnlockAttributeBuffer();


	}

	Vertex* v = 0;
	mMesh->LockVertexBuffer(0,(void**)&v);
	if (v == 0)
	{
		MessageBox(0,"lock boudingbox mesh vertex buffer error","",MB_OK);
		return;
	}
	else
	{
		for (int i = 0; i<8; ++i)
		{
			v[i] = Vertex(boundingbox.corner(i),color);
		}
	}
	mMesh->UnlockVertexBuffer();

	//D3DMATERIAL9 mtrl;
	//memset(&mtrl,0,sizeof(D3DMATERIAL9));
	//D3DXCOLOR dc(color);
	//mtrl.Diffuse = dc;

	mDevice->SetTransform(D3DTS_WORLD,&mWorldIdentity);

	DWORD light;
	DWORD cull;
	mDevice->GetRenderState(D3DRS_LIGHTING,&light);
	mDevice->GetRenderState(D3DRS_CULLMODE,&cull);

	mDevice->SetRenderState(D3DRS_LIGHTING,false);
	mDevice->SetRenderState(D3DRS_ZWRITEENABLE,false);
	mDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true);
	mDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);
	mDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
	mDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	mDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

	mDevice->BeginScene();

	D3DXMATRIX m;
	D3DXMatrixIdentity(&m);
	mDevice->SetTransform(D3DTS_WORLD,&m);
	//mDevice->SetMaterial(&mtrl);
	mMesh->DrawSubset(0);
	//mDevice->DrawIndexedPrimitive(D3DPT_LINELIST,0,0,8,0,12);
	mDevice->SetRenderState(D3DRS_LIGHTING,light);
	mDevice->SetRenderState(D3DRS_CULLMODE,cull);
	mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,false);
	mDevice->SetRenderState(D3DRS_ZWRITEENABLE,true);
	mDevice->EndScene();

}

void createD3DBoundingBoxRenderer(IBoundingBoxRenderer** out,IDirect3DDevice9* device)
{
	if (*out == 0)
	{
		*out = new D3DBoundingBoxRenderer(device);
	}
	else
	{
		(*out)->release();
		*out = new D3DBoundingBoxRenderer(device);
	}
}