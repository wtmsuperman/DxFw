#include <dx/dxfw.h>
#include <node/scene_node.h>
#include <time.h>
#include <dx/dx_logging.h>

DxFw*	gEngine;
Node*	gCamera;

class Entity : public AttachableObject
{
public:
	Entity(DxModel* model)
	{
		this->mXmodel = model;
	}

	void setBoundingbox(const AABB3& boudingbox)
	{
		mBoundingbox = boudingbox;
	}

	AABB3 getWorldAABB()
	{
		if (mParent)
		{
			Matrix4x4 m;
			mParent->generateLocalToWorldMatrix(&m);
			mBoundingbox.transform(m);
		}
		else
		{
			mBoundingbox.transform(Matrix4x4::IDENTITY);
		}
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
	AABB3		mBoundingbox;
};

class GameEntity
{
public:
	SceneNode*	node;
	float		speed;
	bool		isAlive;
	AABB3		boundingbox;
	int			type;
public:
	virtual void update(float delta) = 0;

	virtual bool collisionTest(GameEntity* entity)
	{
		if ( entity->boundingbox.intersects(entity->boundingbox) )
		{
			this->collisionImpl(entity);
			entity->collisionImpl(this);
			return true;
		}
		return false;
	}

	virtual void collisionImpl(GameEntity* entity) = 0;
};

class MyShip : public GameEntity
{
public:
	MyShip(Node* parent)
	{
		node =(SceneNode*) parent->createChild("myship");
		node->attachObject(new Entity(gEngine->getResourceManager()->getResourceGroup("hero")->getModel("ship.x")));
		node->yaw(PI);
		speed = 100.0f;
	}

	virtual void update(float delta)
	{
		//更新状态
	}

	virtual void collisionImpl(GameEntity* entity)
	{
		//实现碰撞
	}
};

MyShip*		gPlayer;

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
	//gEngine->getRenderer()->enableLight(false);
}

void initCamera()
{
	gCamera = new Node;
	gCamera->setPosition(0.0f,50.0f,-40.0f);
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

void processInput(float delta)
{
	IInputSystem* input = gEngine->getInputSystem();

	if (input->keyDown(DIK_F5))
	{
		gEngine->saveScreenshot();
		return;
	}

	Vector3 walk = Vector3::ZERO;
	if (input->keyDown(DIK_W))
	{
		walk.z += gPlayer->speed;
	}
	if (input->keyDown(DIK_S))
	{
		walk.z -= gPlayer->speed;
	}
	if (input->keyDown(DIK_A))
	{
		walk.x -= gPlayer->speed;
	}
	if (input->keyDown(DIK_D))
	{
		walk.x += gPlayer->speed;
	}
	walk *= delta;
	gPlayer->node->translate(walk,Node::TS_LOCAL);
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
	GUISystem* guisys = GUISystem::getSingletonPtr();
	guisys->initOnce(*gEngine);

	loggingInit(guisys);
	
	SceneNode* root = new SceneNode("root");
	root->yaw(PI);
	gPlayer = new MyShip(root);
	
	DxParticleSystem ps;
	loadParticleSystem(&ps,gEngine,"media/particle/ship_gas.lua");

	SceneNode* fire = (SceneNode*)gPlayer->node->createChild("gas");
	fire->attachObject(&ps);
	fire->setPosition(0.0f,0.0f,-8.0);

	MSG msg;
	while (true)
	{
		renderer->clear(true,true,true,0xff000000);
		applyCamera();

		renderer->beginScene();
		renderer->render(root);
		guisys->render();
		renderer->endScene();

		float delta = getTimeSinceLastFrame();
		root->update(delta);
		processInput(delta);
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