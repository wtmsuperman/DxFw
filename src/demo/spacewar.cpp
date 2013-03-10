#include <dx/dxfw.h>
#include <node/scene_node.h>
#include <time.h>

DxFw*	gEngine;
Node*	gCamera;

class Entity : public AttachableObject
{
public:
	Entity(DxModel* model)
	{
		this->mXmodel = model;
	}

	virtual void preRender(DxRenderer* renderer)
	{}

	virtual void onRender(DxRenderer* renderer)
	{
		if (mParent)
		{
			Matrix4x4 m;
			mParent->generateLocalToWorldMatrix(&m);
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

	virtual void postRender(DxRenderer* renderer)
	{}

	virtual void update(float delta)
	{
	}
	
	DxModel*	mXmodel;
};

void initEngine(WinInfo& info)
{
	//for random
	srand((unsigned int)time(0));

	DxParam param = {info.isFullScreen,info.width,info.height,info.hwnd};
	if (!gEngine->initDx(param))
	{
		MessageBox(0,"fuck","",MB_OK);
	}

	if (!gEngine->initInput(info.hwnd,info.hist,false))
	{
		MessageBox(0,"input failed","",MB_OK);
	}

	registAllDefaultAffectos();

	gEngine->getRenderer()->setAsPerspectiveProjection(PI_OVER_2,(float)info.width / (float) info.height,1.0f,1000.0f);
}

void initEnvironment()
{
	D3DLIGHT9 light;
	::ZeroMemory(&light, sizeof(light));
	light.Type      = D3DLIGHT_DIRECTIONAL;
	light.Ambient   = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	light.Diffuse   = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light.Specular  = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
	light.Direction = D3DXVECTOR3(1.0f, -1.0f, 0.0f);

	IDirect3DDevice9* device = gEngine->getDevice();

	device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
	device->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_POINT);

	gEngine->getRenderer()->setLight(0,&light);
}

void initCamera()
{
	gCamera = new Node;
	gCamera->setPosition(0.0f,100.0f,0.0f);
	gCamera->lookAt(0.0f,0.0f,0.0f);
}

void applyCamera()
{
	Matrix4x4 view;
	gCamera->generateWorldToLocalMatrix(&view);
	gEngine->getRenderer()->setViewMatrix(view);
}

void loadResource()
{
	DxResourceGroup* shipGroup = gEngine->getResourceManager()->createResourceGroup("hero");
	shipGroup->loadXModel("ship.x");
}

void releaseAll()
{
	gEngine->release();

	delete gCamera;
	delete gEngine;
}

int WINAPI WinMain(HINSTANCE hist,HINSTANCE phist,LPSTR cmd,int show)
{
	gEngine = new DxFw;

	WinInfo info;
	if (!initWindow(800,600,false,"my game",hist,&info))
	{
		return -1;
	}
	initEngine(info);
	initEnvironment();
	initCamera();
	loadResource();

	DxRenderer* renderer = gEngine->getRenderer();
	IInputSystem* inputSys = gEngine->getInputSystem();

	SceneNode* root = new SceneNode("root");
	root->attachObject(new Entity(gEngine->getResourceManager()->getResourceGroup("hero")->getModel("ship.x")));
	root->yaw(PI);
	
	DxParticleSystem ps;
	loadParticleSystem(&ps,gEngine,"media/particle/fire.particle");

	SceneNode* fire = (SceneNode*)root->createChild("fire");
	fire->attachObject(&ps);
	fire->setPosition(0.0f,0.0f,-10.0);

	MSG msg;
	while (true)
	{
		renderer->clear(true,true,true,0xff000000);
		applyCamera();

		renderer->beginScene();
		renderer->render(root);
		renderer->endScene();

		float delta = getTimeSinceLastFrame();
		root->update(delta);

		inputSys->capture();
		renderer->present();
		messagePump(&msg);
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	releaseAll();
	UnregisterClass(info.className,info.hist);
	return 0;
}