#include <other/skybox.h>

void SkyBox::init(IDirect3DDevice9* device,float size)
{
	this->mDevice = device;
	this->mSize = size;
	//1.�������������㻺��
	mDevice->CreateVertexBuffer( 24 * sizeof(SkyVertex), 0, 
		SkyVertex::FVF, D3DPOOL_MANAGED, &mVertexBuffer, 0 );

	//��һ���ṹ��Ѷ���������׼����
	SkyVertex vertices[] =
    {
		//ǰ����ĸ�����
		{ -mSize/2, -mSize/2,    mSize/2, 0.0f, 1.0f, },
		{ -mSize/2, mSize/2,   mSize/2, 0.0f, 0.0f, },
		{  mSize/2, -mSize/2,    mSize/2, 1.0f, 1.0f, },
		{  mSize/2, mSize/2,   mSize/2, 1.0f, 0.0f, },

		//������ĸ�����
		{  mSize/2, -mSize/2,   -mSize/2, 0.0f, 1.0f, },
		{  mSize/2, mSize/2,  -mSize/2, 0.0f, 0.0f, },
		{ -mSize/2, -mSize/2,   -mSize/2, 1.0f, 1.0f, },
		{ -mSize/2, mSize/2,  -mSize/2, 1.0f, 0.0f, },

		//������ĸ�����
		{ -mSize/2, -mSize/2,   -mSize/2, 0.0f, 1.0f, },
		{ -mSize/2, mSize/2,  -mSize/2, 0.0f, 0.0f, },
		{ -mSize/2, -mSize/2,    mSize/2, 1.0f, 1.0f, },
		{ -mSize/2, mSize/2,   mSize/2, 1.0f, 0.0f, },

		//������ĸ�����
		{ mSize/2, -mSize/2,   mSize/2, 0.0f, 1.0f, },
		{ mSize/2, mSize/2,  mSize/2, 0.0f, 0.0f, },
		{ mSize/2, -mSize/2,  -mSize/2, 1.0f, 1.0f, },
		{ mSize/2, mSize/2, -mSize/2, 1.0f, 0.0f, },

		//������ĸ�����
		{  mSize/2, mSize/2, -mSize/2, 1.0f, 0.0f, },
		{  mSize/2, mSize/2,  mSize/2, 1.0f, 1.0f, },
		{ -mSize/2, mSize/2, -mSize/2, 0.0f, 0.0f, },
		{ -mSize/2, mSize/2,  mSize/2, 0.0f, 1.0f, },

		//������ĸ�����
		{  mSize/2, -mSize/2, -mSize/2, 1.0f, 0.0f, },
		{  mSize/2, -mSize/2,  mSize/2, 1.0f, 1.0f, },
		{ -mSize/2, -mSize/2, -mSize/2, 0.0f, 0.0f, },
		{ -mSize/2, -mSize/2,  mSize/2, 0.0f, 1.0f, },

    };

	//׼����䶥������
    void* pVertices;
	//2.����
    mVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
	//3.���ʡ��ѽṹ���е�����ֱ�ӿ������㻺������
    memcpy( pVertices, vertices, sizeof(vertices) );
	//4.����
	mVertexBuffer->Unlock();
	mPosition = Vector3::ZERO;
}

void SkyBox::load(char *texfiles[])
{	
	for (int i=0; i<6; ++i)
	{
		char* t = texfiles[i];
		HRESULT hr = D3DXCreateTextureFromFile( mDevice , texfiles[i], &mTextures[i] );  
		if (FAILED(hr))
		{
			assert(false);
		}
	}
}

void SkyBox::render()
{
	//mDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);  //��������ɫ��ϵĵ�һ����������ɫֵ�������
	//mDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );   //������ɫ��ϵĵ�һ��������ֵ��ȡ������ɫֵ
	mDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	mDevice->SetRenderState(D3DRS_LIGHTING,false);
	mDevice->SetRenderState(D3DRS_ZENABLE,true);
	mDevice->SetRenderState(D3DRS_ZWRITEENABLE,false);

	Matrix4x4 m;
	m.setupTranslate(mPosition);
	//D3DXMATRIX m;
	//D3DXMatrixIdentity(&m);
	mDevice->SetTransform( D3DTS_WORLD, (D3DXMATRIX*)&m);  //�����������

	mDevice->SetStreamSource(0,mVertexBuffer, 0, sizeof(SkyVertex));    //�Ѱ����ļ�������Ϣ�Ķ��㻺�����Ⱦ��ˮ�������  
	mDevice->SetFVF(SkyVertex::FVF);  //����FVF�����ʽ

	//һ��forѭ������6������Ƴ���
	for(int i =0; i<6; i++)
	{
		mDevice->SetTexture(0, mTextures[i]);
		mDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, i*4, 2);
	}
	mDevice->SetRenderState(D3DRS_LIGHTING,true);
	mDevice->SetRenderState(D3DRS_ZENABLE,true);
	mDevice->SetRenderState(D3DRS_ZWRITEENABLE,true);
	mDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
}

void SkyBox::release()
{
	for (int i=0; i<6; ++i)
	{
		mTextures[i]->Release();
	}
	mVertexBuffer->Release();
}

void SkyBox::translate(float x,float y,float z)
{
	mPosition += Vector3(x,y,z);
}