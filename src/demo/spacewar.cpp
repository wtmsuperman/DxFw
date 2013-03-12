#include <dx/dxfw.h>
#include <node/scene_node.h>
#include <time.h>
#include <dx/dx_logging.h>

class BulletManager;

DxFw*	gEngine;
Node*	gCamera;
BulletManager* gBulletMgr;

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

	// transform to world space
	virtual AABB3 getWorldAABB3()
	{
		Matrix4x4 m;
		AABB3 box;
		m.setupLocalToParent(node->getDerivedPosition(),node->getDerivedOrientation());
		box.setToTransformedBox(boundingbox,m);
		return box;
	}
};

struct BulletVertex
{
	float x,y,z;
	DWORD color;
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

struct BulletAttribute
{
	float size;
	float speed;
	float harm;
	int category;
	DWORD color;
};

class Bullet
{
public:
	Vector3	direction;
	int category; //通过该值，决定使用的贴图
	float size;
	float speed;
	float harm;
	bool isAlive;
	float time;
	Vector3	position; //世界坐标

	Bullet()
	{
		time = 1.0f;
	}

	// default implements
	virtual void update(float delta)
	{
		if (time - delta <= 0.0f)
		{
			isAlive = false;
		}
		position += direction * speed * delta;
		time -= delta;
	}

	// calculate harm
	virtual float calcHarm()
	{
		return harm;
	}

	virtual bool collisionTest(GameEntity* entity)
	{
		// transform both to world space
		Vector3 epos = entity->getWorldAABB3().center();
		if (position.squaredDistance(epos) < size * size)
		{
			collisionImpl(entity);
			return true;
		}
		return false;
	}

	virtual void collisionImpl(GameEntity* entity)
	{
		isAlive = false;
	}
};

class BulletManager : public AttachableObject
{
private:
	typedef std::list<Bullet*>					Bullets;
	typedef std::map<int,Bullets>				BulletsMap;
	typedef Bullets::iterator					BulletsIter;
	typedef BulletsMap::iterator				BulletsMapIter;
	typedef std::map<int,DxTexture*>			TextureMap;

	BulletsMap									mMyBullets;
	BulletsMap									mEnemyBullets;
	TextureMap									mTextureMap;
	DxResourceGroup*							mResourceGroup;
private:
	//子弹的数量一般不会太多，没有必要采用batch
	IDirect3DVertexBuffer9*						mVertexBuffer;
	static const int							MAX_BUFFER_SIZE = 512;

private:
	void renderBullets(DxRenderer* renderer,BulletsMap& bullets)
	{
		BulletsMapIter end = bullets.end();
		for (BulletsMapIter iter=bullets.begin(); iter!=end; ++iter)
		{
			Bullets& bulletlist = iter->second;
			if (bulletlist.empty())
				continue;
			else
			{
				BulletVertex* v;
				IDirect3DDevice9* device = renderer->getDevice();
				renderer->applyTexture(0,mTextureMap[iter->first]);
				device->SetStreamSource(0,mVertexBuffer,0,sizeof(BulletVertex));
				//不为空，第一个一定有值，取出size值
				device->SetRenderState(D3DRS_POINTSIZE,FtoDW((*bulletlist.begin())->size));
				UINT size = bulletlist.size();
				UINT bufferSize = size * sizeof(BulletVertex);
				mVertexBuffer->Lock(0,bufferSize,(void**)&v,0);
				BulletsIter bulletEnd = bulletlist.end();
				for (BulletsIter biter=bulletlist.begin(); biter!=bulletEnd; ++biter)
				{
					v->x = (*biter)->position.x;
					v->y = (*biter)->position.y;
					v->z = (*biter)->position.z;
					v->color = 0xffffffff;
					++v;
				}
				device->DrawPrimitive(D3DPT_POINTLIST,0,size);
				mVertexBuffer->Unlock();
			}
		}
	}

	void update(BulletsMap bullets,float delta)
	{
		BulletsMapIter end0 = bullets.end();
		for (BulletsMapIter iter=bullets.begin(); iter!=end0; ++iter)
		{
			Bullets& bulletlist = iter->second;
			if (bulletlist.empty())
				continue;
			else
			{
				logToScreen("bullet","%d",bulletlist.size());
				BulletsIter bulletEnd = bulletlist.end();
				for (BulletsIter biter=bulletlist.begin(); biter!=bulletEnd;)
				{
					Bullet* bullet = *biter;
					bullet->update(delta);
					if (!bullet->isAlive)
					{
						biter = bulletlist.erase(biter);
						//delete bullet;
					}
					else
					{
						++biter;
					}
				}
			}
		}
	}

	void release(BulletsMap bullets)
	{
		BulletsMapIter end0 = bullets.end();
		//删除所有的子弹
		for (BulletsMapIter iter=bullets.begin(); iter!=end0; ++iter)
		{
			Bullets& bulletlist = iter->second;
			if (bulletlist.empty())
				continue;
			else
			{
				BulletsIter bulletEnd = bulletlist.end();
				for (BulletsIter biter=bulletlist.begin(); biter!=bulletEnd;++biter)
				{
					delete (*biter);
				}
			}
		}
		//清空map
		bullets.clear();
	}

	void collisionTest(BulletsMap bullets,GameEntity* entity)
	{
		BulletsMapIter end0 = bullets.end();
		for (BulletsMapIter iter=bullets.begin(); iter!=end0; ++iter)
		{
			Bullets& bulletlist = iter->second;
			if (bulletlist.empty())
				continue;
			else
			{
				BulletsIter bulletEnd = bulletlist.end();
				for (BulletsIter biter=bulletlist.begin(); biter!=bulletEnd; ++biter)
				{
					(*biter)->collisionTest(entity);
				}
			}
		}
	}
	
public:
	BulletManager()
	{
		mResourceGroup = 0;
		mVertexBuffer = 0;
	}

	~BulletManager()
	{
		safe_delete(mResourceGroup);
		safe_Release(mVertexBuffer);
		release(mMyBullets);
		release(mEnemyBullets);
	}

	bool init(DxFw* engine)
	{
		mResourceGroup = engine->getResourceManager()->createResourceGroup("bullets");
		if (mResourceGroup == 0)
			return false;
		if (FAILED( engine->getDevice()->CreateVertexBuffer(MAX_BUFFER_SIZE * sizeof(ParticleVertex)
		,D3DUSAGE_DYNAMIC | D3DUSAGE_POINTS | D3DUSAGE_WRITEONLY
		,BulletVertex::FVF
		,D3DPOOL_DEFAULT
		,&mVertexBuffer
		,0)))
		{
			return false;
		}
		return true;
	}

	void load(const char* scriptFile)
	{
		lua_State* L = lua_open();
		luaL_loadfile(L,scriptFile);
		lua_pcall(L,0,0,0);
		lua_getglobal(L,"bullets");

		lua_pushnil(L);
		while (lua_next(L,1) != 0)
		{
			// key at -2,value at -1
			int category = (int)lua_tonumber(L,-2);
			mTextureMap[category] = mResourceGroup->loadTexture(lua_tostring(L,-1));
			lua_pop(L,1);
		}

		lua_close(L);
	}

	void preRender(DxRenderer* renderer)
	{
	}
	void onRender(DxRenderer* renderer)
	{
		DWORD light;
		renderer->getDevice()->GetRenderState(D3DRS_LIGHTING,&light);
		renderer->setRenderState(D3DRS_LIGHTING,false);
		renderer->setRenderState(D3DRS_ZWRITEENABLE,false);
		renderer->setRenderState(D3DRS_POINTSPRITEENABLE,true);
		renderer->setRenderState(D3DRS_POINTSCALEENABLE,true);
		renderer->setRenderState(D3DRS_POINTSIZE_MIN,FtoDW(0.f));

		renderer->setRenderState(D3DRS_POINTSCALE_A,FtoDW(0.0f));
		renderer->setRenderState(D3DRS_POINTSCALE_B,FtoDW(0.0f));
		renderer->setRenderState(D3DRS_POINTSCALE_C,FtoDW(1.0f));

		renderer->setTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
		renderer->setTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
		renderer->enableTransparent();
		///////////////////////////////////////////////////////////////////////////

		renderBullets(renderer,mMyBullets);
		renderBullets(renderer,mEnemyBullets);

		///////////////////////////////////////////////////////////////////////////
		renderer->setRenderState(D3DRS_ZWRITEENABLE,true);
		renderer->setRenderState(D3DRS_POINTSPRITEENABLE,false);
		renderer->setRenderState(D3DRS_POINTSCALEENABLE,false);
		renderer->setRenderState(D3DRS_LIGHTING,light);
		renderer->disableTransparent();
	}
	void postRender(DxRenderer* renderer)
	{
	}

	void update(float delta)
	{
		logToScreen("bullet","%d",mMyBullets[1].size());
		//update 己方的子弹
		update(mMyBullets,delta);
		//update 地方子弹
		update(mEnemyBullets,delta);
	}

	void addBullet(Bullet* bullet,int type)
	{
		//敌人还是自己
		if (type == 0)
		{
			mEnemyBullets[bullet->category].push_back(bullet);
		}
		else if (type == 1)
		{
			mMyBullets[bullet->category].push_back(bullet);
			return;
		}
	}

	void collisionTest(GameEntity* entity,int type)
	{
		if (type == 0)
		{
			collisionTest(mEnemyBullets,entity);
		}
		else if (type == 1)
		{
			collisionTest(mMyBullets,entity);
		}
	}
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
	delete gBulletMgr;
	delete gCamera;
	gEngine->release();
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
	if (input->keyDown(DIK_J))
	{
		Bullet* b = new Bullet;
		b->isAlive = true;
		b->speed = 100.f;
		b->direction = Vector3(0.0f,0.0f,1.0f);
		b->position = gPlayer->node->getDerivedPosition();
		b->size = 1.0f;
		b->category = 1;
		gBulletMgr->addBullet(b,1);
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

	gBulletMgr = new BulletManager;
	gBulletMgr->init(gEngine);
	gBulletMgr->load("media/bullet/all.lua");
	root->attachObject(gBulletMgr);

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