#include <other/skybox.h>

void SkyBox::init(IDirect3DDevice9* device,float size)
{
	this->mDevice = device;
	this->mSize = size;
	//1.创建。创建顶点缓存
	mDevice->CreateVertexBuffer( 24 * sizeof(SkyVertex), 0, 
		SkyVertex::FVF, D3DPOOL_MANAGED, &mVertexBuffer, 0 );

	//用一个结构体把顶点数据先准备好
	SkyVertex vertices[] =
    {
		//前面的四个顶点
		{ -mSize/2, -mSize/2,    mSize/2, 0.0f, 1.0f, },
		{ -mSize/2, mSize/2,   mSize/2, 0.0f, 0.0f, },
		{  mSize/2, -mSize/2,    mSize/2, 1.0f, 1.0f, },
		{  mSize/2, mSize/2,   mSize/2, 1.0f, 0.0f, },

		//背面的四个顶点
		{  mSize/2, -mSize/2,   -mSize/2, 0.0f, 1.0f, },
		{  mSize/2, mSize/2,  -mSize/2, 0.0f, 0.0f, },
		{ -mSize/2, -mSize/2,   -mSize/2, 1.0f, 1.0f, },
		{ -mSize/2, mSize/2,  -mSize/2, 1.0f, 0.0f, },

		//左面的四个顶点
		{ -mSize/2, -mSize/2,   -mSize/2, 0.0f, 1.0f, },
		{ -mSize/2, mSize/2,  -mSize/2, 0.0f, 0.0f, },
		{ -mSize/2, -mSize/2,    mSize/2, 1.0f, 1.0f, },
		{ -mSize/2, mSize/2,   mSize/2, 1.0f, 0.0f, },

		//右面的四个顶点
		{ mSize/2, -mSize/2,   mSize/2, 0.0f, 1.0f, },
		{ mSize/2, mSize/2,  mSize/2, 0.0f, 0.0f, },
		{ mSize/2, -mSize/2,  -mSize/2, 1.0f, 1.0f, },
		{ mSize/2, mSize/2, -mSize/2, 1.0f, 0.0f, },

		//上面的四个顶点
		{  mSize/2, mSize/2, -mSize/2, 1.0f, 0.0f, },
		{  mSize/2, mSize/2,  mSize/2, 1.0f, 1.0f, },
		{ -mSize/2, mSize/2, -mSize/2, 0.0f, 0.0f, },
		{ -mSize/2, mSize/2,  mSize/2, 0.0f, 1.0f, },

		//下面的四个顶点
		{  mSize/2, -mSize/2, -mSize/2, 1.0f, 0.0f, },
		{  mSize/2, -mSize/2,  mSize/2, 1.0f, 1.0f, },
		{ -mSize/2, -mSize/2, -mSize/2, 0.0f, 0.0f, },
		{ -mSize/2, -mSize/2,  mSize/2, 0.0f, 1.0f, },

    };

	//准备填充顶点数据
    void* pVertices;
	//2.加锁
    mVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
	//3.访问。把结构体中的数据直接拷到顶点缓冲区中
    memcpy( pVertices, vertices, sizeof(vertices) );
	//4.解锁
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
	//mDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);  //将纹理颜色混合的第一个参数的颜色值用于输出
	//mDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );   //纹理颜色混合的第一个参数的值就取纹理颜色值
	mDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	mDevice->SetRenderState(D3DRS_LIGHTING,false);
	mDevice->SetRenderState(D3DRS_ZENABLE,true);
	mDevice->SetRenderState(D3DRS_ZWRITEENABLE,false);

	Matrix4x4 m;
	m.setupTranslate(mPosition);
	//D3DXMATRIX m;
	//D3DXMatrixIdentity(&m);
	mDevice->SetTransform( D3DTS_WORLD, (D3DXMATRIX*)&m);  //设置世界矩阵

	mDevice->SetStreamSource(0,mVertexBuffer, 0, sizeof(SkyVertex));    //把包含的几何体信息的顶点缓存和渲染流水线相关联  
	mDevice->SetFVF(SkyVertex::FVF);  //设置FVF灵活顶点格式

	//一个for循环，将6个面绘制出来
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