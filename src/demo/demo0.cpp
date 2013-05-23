#include <dx/dxfw.h>
#include <node/scene_node.h>
#include <time.h>
#include <dx/dx_logging.h>

#include <map>
#include <list>
#include <dx/boundingBoxRenderer.h>

class Bullet;
class BulletManager;
class MyShip;
class EnemyManager;

DxFw*	gEngine;
Node*	gCamera;

BulletManager* gBulletMgr;
IBoundingBoxRenderer* gBoxRenderer;
MyShip*		gPlayer;
EnemyManager* gEnemyMgr;

int gPoint;
float gLiveTime;

enum GAME_STATE
{
	GAME_PAUSE,
	GAME_RUN,
	GAME_MENU0,
	GAME_MENU1,
	GAME_MENU2,
	GAME_MENU3,
	GAME_QUIT,
};

GAME_STATE gGameState;

class Entity : public AttachableObject
{
public:
	Entity(DxModel* model)
	{
		this->mXmodel = model;

		//使用的是X file，因此暂时使用directx提供的方式计算boundingbox
		BYTE* byte;
		model->mesh->meshData->LockVertexBuffer(0,(void**)&byte);
		D3DXVECTOR3 max,min;
		D3DXComputeBoundingBox((D3DXVECTOR3*)byte,model->mesh->meshData->GetNumVertices(),D3DXGetFVFVertexSize(model->mesh->meshData->GetFVF()),&min,&max);
		model->mesh->meshData->UnlockVertexBuffer();
		mBoundingbox.max = Vector3(max.x,max.y,max.z);
		mBoundingbox.min = Vector3(min.x,min.y,min.z);
		this->mIsDraw = true;
	}

	void setModel(DxModel* model)
	{
		this->mXmodel = model;

		//使用的是X file，因此暂时使用directx提供的方式计算boundingbox
		BYTE* byte;
		model->mesh->meshData->LockVertexBuffer(0,(void**)&byte);
		D3DXVECTOR3 max,min;
		D3DXComputeBoundingBox((D3DXVECTOR3*)byte,model->mesh->meshData->GetNumVertices(),D3DXGetFVFVertexSize(model->mesh->meshData->GetFVF()),&min,&max);
		model->mesh->meshData->UnlockVertexBuffer();
		mBoundingbox.max = Vector3(max.x,max.y,max.z);
		mBoundingbox.min = Vector3(min.x,min.y,min.z);
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
			AABB3 box;
			box.setToTransformedBox(mBoundingbox,m);
			return box;
		}
		else
		{
			AABB3 box;
			box.setToTransformedBox(mBoundingbox,Matrix4x4::IDENTITY);
			return box;
		}
	}

	virtual void preRender(DxRenderer* renderer)
	{}

	virtual void onRender(DxRenderer* renderer)
	{
		if (!mIsDraw)
		{
			return;
		}

		AABB3 box;
		if (mParent)
		{
			Matrix4x4 m;
			mParent->generateLocalToWorldMatrix(&m);
			box.setToTransformedBox(mBoundingbox,m);
			renderer->setWorldTransform(m);
		}
		else
		{
			box.setToTransformedBox(mBoundingbox,Matrix4x4::IDENTITY);
			renderer->setWorldTransform(Matrix4x4::IDENTITY);
		}

	//renderer->setRenderState(D3DRS_LIGHTING,true);
	
		unsigned int faceNum = mXmodel->faceNum;
		for (unsigned int i=0; i < faceNum; ++i)
		{
			renderer->setMaterial(mXmodel->mtrls[i]);
			mXmodel->mesh->meshData->DrawSubset(i);
		}

		gBoxRenderer->draw(box);
	}

	virtual void postRender(DxRenderer* renderer)
	{}

	virtual void update(float delta)
	{
	}
	
	DxModel*	mXmodel;
	AABB3		mBoundingbox;
	bool		mIsDraw;
};

class GameEntity
{
public:
	SceneNode*	node;
	float		speed;
	bool		isAlive;
	Entity*		entity;
public:

	virtual void update(float delta) = 0;

	virtual bool hitByBullet(Bullet* bullet)
	{
		return true;
	}

	virtual bool collisionTest(GameEntity* entity)
	{
		if ( this->entity->getWorldAABB().intersects(entity->getWorldAABB()) )
		{
			this->collisionImpl(entity);
			entity->collisionImpl(this);
			return true;
		}
		return false;
	}

	virtual void collisionImpl(GameEntity* entity) = 0;

	// transform to world space
	virtual AABB3 getWorldAABB()
	{
		return entity->getWorldAABB();
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
	int id;
	DWORD color;
	DxTexture*	tex;
};

class Bullet
{
public:
	Vector3	direction;
	//子弹类型
	int id;
	BulletAttribute* attribute;
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
		position += direction * attribute->speed * delta;
		time -= delta;
	}

	// calculate harm
	virtual float calcHarm()
	{
		return attribute->harm;
	}

	virtual bool collisionTest(GameEntity* entity)
	{
		// transform both to world space
		if (entity->getWorldAABB().contains(position))
		{
			collisionImpl(entity);
			entity->hitByBullet(this);
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
	typedef std::map<int,BulletAttribute>		BulletAttributeMap;

	BulletsMap									mMyBullets;
	BulletsMap									mEnemyBullets;
	BulletAttributeMap							mBulletAttributeMap;
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
				BulletAttribute& attr = mBulletAttributeMap[iter->first];
				renderer->applyTexture(0,attr.tex);
				device->SetStreamSource(0,mVertexBuffer,0,sizeof(BulletVertex));
				device->SetRenderState(D3DRS_POINTSIZE,FtoDW(attr.size));
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

	void update(BulletsMap& bullets,float delta)
	{
		BulletsMapIter end0 = bullets.end();
		for (BulletsMapIter iter=bullets.begin(); iter!=end0; ++iter)
		{
			Bullets& bulletlist = iter->second;
			logToScreen("bullet","%d",bulletlist.size());
			if (bulletlist.empty())
				continue;
			else
			{
				BulletsIter bulletEnd = bulletlist.end();
				for (BulletsIter biter=bulletlist.begin(); biter!=bulletEnd;)
				{
					Bullet* bullet = *biter;
					bullet->update(delta);
					if (!bullet->isAlive)
					{
						biter = bulletlist.erase(biter);
						delete bullet;
					}
					else
					{
						++biter;
					}
				}
			}
		}
	}

	void release(BulletsMap& bullets)
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

	void collisionTest(BulletsMap& bullets,GameEntity* entity)
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
		if(luaL_loadfile(L,scriptFile))
		{
			assert(false && "load bullet script failed");
		}
		lua_pcall(L,0,0,0);
		lua_getglobal(L,"bullets");

		assert(lua_istable(L,-1) && "load bullet script");

		lua_pushnil(L);
		while (lua_next(L,1) != 0)
		{
			// key at -2,value at -1
			
			assert(lua_istable(L,-1) && "load bullet script");
			BulletAttribute attr;
			//texture size harm speed id必须有
			
			//texture
			lua_getfield(L,-1,"texture");
			assert(lua_isstring(L,-1) && "load bullet texture");
			attr.tex = mResourceGroup->loadTexture(lua_tostring(L,-1));
			lua_pop(L,1);
			//size
			lua_getfield(L,-1,"size");
			assert(lua_isnumber(L,-1) && "load bullet size");
			attr.size = (float)lua_tonumber(L,-1);
			lua_pop(L,1);
			//speed
			lua_getfield(L,-1,"speed");
			assert(lua_isnumber(L,-1) && "load bullet speed");
			attr.speed = (float)lua_tonumber(L,-1);
			lua_pop(L,1);
			//id
			lua_getfield(L,-1,"id");
			assert(lua_isnumber(L,-1) && "load bullet id");
			attr.id = (int)lua_tonumber(L,-1);
			lua_pop(L,1);
			//harm
			lua_getfield(L,-1,"harm");
			assert(lua_isnumber(L,-1) && "load bullet harm");
			attr.harm = (float)lua_tonumber(L,-1);
			lua_pop(L,1);
			//color
			//color可以没有
			lua_getfield(L,-1,"color");
			if(lua_istable(L,-1))
			{
				lua_rawgeti(L,-1,1); //a
				lua_rawgeti(L,-2,2); //r
				lua_rawgeti(L,-3,3); //g
				lua_rawgeti(L,-4,4); //b
				float a = (float)lua_tonumber(L,-4);
				float r = (float)lua_tonumber(L,-3);
				float g = (float)lua_tonumber(L,-2);
				float b = (float)lua_tonumber(L,-1);
				attr.color = D3DCOLOR_COLORVALUE(r,g,b,a);
				lua_pop(L,1); // pop b
				lua_pop(L,1); // pop g
				lua_pop(L,1); // pop r
				lua_pop(L,1); // pop a
			}
			else
			{
				//default color
				attr.color = 0xffffffff; 
			}
			lua_pop(L,1);
			mBulletAttributeMap[attr.id] = attr;
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
		renderer->setWorldTransform(Matrix4x4::IDENTITY);
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
			mEnemyBullets[bullet->id].push_back(bullet);
		}
		else if (type == 1)
		{
			mMyBullets[bullet->id].push_back(bullet);
			return;
		}
	}

	void initBullet(Bullet* bullet,int id)
	{
		BulletAttribute& attr = mBulletAttributeMap[id];
		bullet->attribute = &attr;
		bullet->id = id;
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

class Weapon : public AttachableObject
{
public:
	//射击延迟
	float fireDelay;
	//重复次数
	int repeatTimes;
	//重复延迟
	float repeatDelay;
	//剩余射击次数
	int repeatRemains;
	//射击延迟剩余
	float fireDelayRemains;
	//重复射击延迟剩余
	float repeatDelayRemains;
	//可以进行射击
	bool ready;
	//进行射击
	bool firing;

	Weapon(float FireDelay,int RepeatTimes,float RepeatDelay)
	{
		fireDelay = FireDelay;
		repeatTimes = RepeatTimes;
		repeatDelay = RepeatDelay;
		fireDelayRemains = FireDelay;
		repeatRemains = RepeatTimes;
		repeatDelayRemains = RepeatDelay;
		ready = true;
		firing = false;
	}

	virtual void fire()
	{
		firing = true;
	}

	// add bullets to mgs
	// 不同的武器有不同的发射方式，重载这个方法进行实现
	virtual void createBullets()
	{
		Bullet* b = new Bullet;
		b->position = getParent()->getDerivedPosition();
		b->direction = Vector3(0.0f,0.0f,1.0);
		gBulletMgr->initBullet(b,1);
		gBulletMgr->addBullet(b,1);

		Bullet* b0 = new Bullet;
		b0->position = getParent()->getDerivedPosition();
		b0->direction = Vector3(-0.2f,0.0f,0.9f);
		gBulletMgr->initBullet(b0,1);
		gBulletMgr->addBullet(b0,1);

		Bullet* b1 = new Bullet;
		b1->position = getParent()->getDerivedPosition();
		b1->direction = Vector3(0.2f,0.0f,0.9f);
		gBulletMgr->initBullet(b1,1);
		gBulletMgr->addBullet(b1,1);
	}

	virtual void update(float delta)
	{
			//发射子弹
		logToScreen("fire","%d,%d",firing,ready);
		logToScreen("fire delay","%f,%f",fireDelayRemains,repeatDelayRemains);

		if (firing)
		{
			if(ready)
			{
				createBullets();
				ready = false;
				repeatRemains = repeatTimes;
			}
			if (repeatRemains > 0)
			{
				if (repeatDelayRemains > 0.0f)
				{
					repeatDelayRemains -= delta;
				}
				else
				{
					repeatDelayRemains = repeatDelay;
					--repeatRemains;
					createBullets();
				}
			}
			else
			{
				firing = false;
			}
		}
		

		if (!ready)
		{
			if (fireDelayRemains > 0.0f)
			{
				fireDelayRemains -= delta;
			}
			else
			{
				fireDelayRemains = fireDelay;
				ready = true;
			}
		}
	}
};

class MyShip : public GameEntity
{
public:
	MyShip(Node* parent)
		:mWeapons(4)
	{
		node =(SceneNode*) parent->createChild("myship");
		entity = new Entity(gEngine->getResourceManager()->getResourceGroup("hero")->getModel("ship.x"));
		node->attachObject(entity);
		node->yaw(PI);
		speed = 100.0f;
		mHp = 10;
	}

	~MyShip()
	{
		delete entity;
		size_t size = mWeapons.size();
		for (size_t i=0; i<size; ++i)
		{
			delete mWeapons[i];
		}
	}

	virtual void update(float delta)
	{
		//更新状态
		logToScreen("HP","%d",mHp);
	}

	virtual void collisionImpl(GameEntity* entity)
	{
		//实现碰撞
		mHp -= 1;
	}

	virtual bool hitByBullet(Bullet* bullet)
	{	
		mHp -= 1;
		return true;
	}

	void addWeapon(Weapon* weapon,int i)
	{
		char buffer[512];
		sprintf_s(buffer,"weapon_%d",mWeapons.size());
		SceneNode* weaponNode = (SceneNode*) node->createChild(buffer);
		weaponNode->attachObject(weapon);
		mWeapons[i] = weapon;
	}

	Weapon* getWeapon(int i)
	{
		return mWeapons[i];
	}

public:
	//主角武器数量一般不会多，直接使用vector存储即可
	std::vector<Weapon*>	mWeapons;
	int						mHp;
};

class Enemy : public GameEntity
{
public:
	Enemy(Node* parent,int i)
	{
		char buffer[64];
		sprintf_s(buffer,"enemy%d",i);
		entity = new Entity(gEngine->getResourceManager()->getResourceGroup("hero")->getModel("ship.x"));
		node =(SceneNode*) parent->createChild(buffer);
		node->attachObject(entity);
	}

	~Enemy()
	{
		delete entity;
	}

	virtual bool hitByBullet(Bullet* bullet)
	{	
		mHp -= 1;
		if (mHp <= 0)
		{
			isAlive = false;
			switch (mType)
			{
			case 0:
				node->attachObject(gEngine->getParticleSystemManager()->createParticleSystem("media/particle/fire.lua"));
				gPoint += 100;
				break;
			case 1:
				node->attachObject(gEngine->getParticleSystemManager()->createParticleSystem("media/particle/fire.lua"));
				gPoint += 200;
				break;
			case 2:
				node->attachObject(gEngine->getParticleSystemManager()->createParticleSystem("media/particle/fire.lua"));
				gPoint += 300;
				break;
			case 3:
				node->attachObject(gEngine->getParticleSystemManager()->createParticleSystem("media/particle/fire.lua"));
				gPoint += 300;
				break;
			case 4:
				node->attachObject(gEngine->getParticleSystemManager()->createParticleSystem("media/particle/fire.lua"));
				gPoint += 400;
				break;
			case 5:
				node->attachObject(gEngine->getParticleSystemManager()->createParticleSystem("media/particle/fire.lua"));
				gPoint += 50;
				break;
			}
			
		}
		return true;
	}

	virtual void update(float delta)
	{
		//更新状态
		Vector3& pos = node->getDerivedPosition();
		if (pos.x < -500.0f || pos.x > 500.0f || pos.z < -400.0f || pos.z > 400.0f)
		{
			isAlive = false;
		}

		if (mType == 1)
		{
			const Vector3& p = gPlayer->node->getDerivedPosition();
			const Vector3& e = this->node->getDerivedPosition();
			if (e.z > p.z )
			{
				float tanTheta = (p.x - e.x) / (p.z - e.z);
				float theta = atanf(tanTheta);
				sinCos(theta,&mDir.x,&mDir.z);
			}
			else
			{
				mDir = Vector3::UNIT_Z;
			}
		}

		if (mType == 3)
		{
			mTimer0 += delta;
			if (mTimer0 > 0.5f)
			{
				mDir = Vector3::UNIT_Z;
			}
		}

		if (mType == 4)
		{

		}

		if (mType == 5)
		{
			long r = randl(0,2);
			float f = randf(1.0f,5.0f);
			if (r == 0)
			{
				node->yaw(delta * f);
			}else if (r == 1)
			{
				node->roll(delta * f);
			}else if (r == 2)
			{
				node->pitch(delta * f);
			}
		}

		Vector3 walk = mDir * (speed * delta);
		node->translate(walk,Node::TS_PARENT);
	}

	virtual void collisionImpl(GameEntity* entity)
	{
		//实现碰撞
		isAlive = false;
		node->attachObject(gEngine->getParticleSystemManager()->createParticleSystem("media/particle/fire.lua"));
	}

	void enableDraw(bool enable)
	{
		this->entity->mIsDraw = enable;
	}

	void setType(int type)
	{
		mType = type;
		if (type == 0) // 随机敌人
		{
			speed = -130.0f;
			entity->setModel(gEngine->getResourceManager()->getResourceGroup("hero")->getModel("ship.x"));
			mHp = 1;
		}
		if (type == 1) //追踪导弹
		{
			speed = -90.0f;
			entity->setModel(gEngine->getResourceManager()->getResourceGroup("hero")->getModel("ship.x"));
			mHp = 1;
		}
		if (type == 2) //交叉6个敌人
		{
			speed = -140.0f;
			entity->setModel(gEngine->getResourceManager()->getResourceGroup("hero")->getModel("ship.x"));
			mHp = 1;
		}
		if (type == 3) //折线敌人
		{
			speed = -200.0f;
			entity->setModel(gEngine->getResourceManager()->getResourceGroup("hero")->getModel("ship.x"));
			mHp = 1;
		}
		if (type == 4) //发1发子弹的敌人
		{
			speed = -100.0f;
			entity->setModel(gEngine->getResourceManager()->getResourceGroup("hero")->getModel("ship.x"));
		}
		if (type == 5) //陨石
		{
			speed = -50.0f;
			entity->setModel(gEngine->getResourceManager()->getResourceGroup("hero")->getModel("ship.x"));
			mHp = 4;
		}
	}

	void reset(int type)
	{
		this->node->detachAll();
		this->node->attachObject(entity);
		this->node->setOrientation(Quaternion::IDENTITY);
		mTimer0 = 0.0f;
		mTimer1 = 0.0f;
		setType(type);
	}
public:
	Vector3		mDir;
	int			mType;
	float		mTimer0;
	float		mTimer1;
	int			mHp;
};

class EnemyManager
{
public:
	EnemyManager(Node* parent)
	{
		for (int i=0; i<40; ++i)
		{
			Enemy* e = new Enemy(parent,i);
			mFrees.push_back(e);
			mPool.push_back(e);
			e->enableDraw(false);
		}
	}

	~EnemyManager()
	{
		for (int i=0; i<40; ++i)
		{
			delete mPool[i];
		}
	}

	void addTwo()
	{
		if (mFrees.size() < 6)
		{
			return;
		}

		for (int i=0; i<3; ++i)
		{
			if (mFrees.empty())
			{
				return;
			}

			Enemy* enemy = mFrees.front();
			mActives.push_back(mFrees.front());
			mFrees.pop_front();

			float x = 150.0f + i * 15.0f;
			enemy->node->setPosition(x,0.0f,100.0f);
			enemy->isAlive= true;
			enemy->mDir.x = 0.5f;
			enemy->mDir.z = 0.3f;

			enemy->reset(2);
			enemy->enableDraw(true);
		}

		for (int i=0; i<3; ++i)
		{
			if (mFrees.empty())
			{
				return;
			}

			Enemy* enemy = mFrees.front();
			mActives.push_back(mFrees.front());
			mFrees.pop_front();

			float x = -(150.0f + i * 15.0f);
			enemy->node->setPosition(x,0.0f,100.0f);
			enemy->isAlive= true;
			enemy->mDir.x = -0.5f;
			enemy->mDir.z = 0.4f;

			enemy->reset(2);
			enemy->enableDraw(true);
		}
	}

	void addZero()
	{
		if (mFrees.empty())
		{
			return;
		}

		Enemy* enemy = mFrees.front();
		mActives.push_back(mFrees.front());
		mFrees.pop_front();

		float x = randf(-90.0f,90.0f);
		enemy->node->setPosition(x,0.0f,200.0f);
		enemy->isAlive= true;
		const Vector3& p = gPlayer->node->getDerivedPosition();
		const Vector3& e = enemy->node->getDerivedPosition();
		float tanTheta = (p.x - e.x) / (p.z - e.z);
		float theta = atanf(tanTheta);
		sinCos(theta,&enemy->mDir.x,&enemy->mDir.z);

		enemy->reset(0);
		enemy->enableDraw(true);
	}

	void addOne()
	{
		int num = (int) randl(1,3);
		if (mFrees.size() < num)
		{
			return;
		}
		for (int i=0; i<num; ++i)
		{
			Enemy* enemy = mFrees.front();
			mActives.push_back(mFrees.front());
			mFrees.pop_front();

			float x = randf(-100.0f,100.0f);
			float z = randf(140.0f,200.0f);
			enemy->node->setPosition(x,0.0f,z);
			enemy->isAlive= true;
			enemy->reset(1);
			enemy->enableDraw(true);
		}
	}

	void addThree()
	{
		int num = 4;
		if (mFrees.size() < num)
		{
			return;
		}
		for (int i=0; i<2; ++i)
		{
			Enemy* enemy = mFrees.front();
			mActives.push_back(mFrees.front());
			mFrees.pop_front();
	
			float x = -50.0f - i*30.0f;
			enemy->node->setPosition(x,0.0f,100.0f);
			enemy->isAlive= true;
			enemy->reset(3);
			enemy->enableDraw(true);
			enemy->mDir = -Vector3::UNIT_X;
		}
		for (int i=0; i<2; ++i)
		{
			Enemy* enemy = mFrees.front();
			mActives.push_back(mFrees.front());
			mFrees.pop_front();
	
			float x = 50.0f + i*30.0f;
			enemy->node->setPosition(x,0.0f,100.0f);
			enemy->isAlive= true;
			enemy->reset(3);
			enemy->enableDraw(true);
			enemy->mDir = Vector3::UNIT_X;
		}
	}

	void addFive()
	{
		int num = (int) randl(3,6);
		if (mFrees.size() < num)
		{
			return;
		}
		for (int i=0; i<num; ++i)
		{
			Enemy* enemy = mFrees.front();
			mActives.push_back(mFrees.front());
			mFrees.pop_front();
	
			float x = randf(-40.0f,40.0f);
			enemy->node->setPosition(x,0.0f,200.0f);
			enemy->isAlive= true;
			enemy->reset(5);
			enemy->enableDraw(true);
			enemy->mDir = Vector3::UNIT_Z;
		}
	}

	void update(float delta)
	{
		logToScreen("current enemy","%d",mActives.size());
		static float counter0 = 0.0f;
		static float counter1 = 0.0f;
		static float counter2 = 0.0f;
		static float counter3 = 0.0f;
		static float counter4 = 0.0f;
		static float counter5 = 0.0f;

		if (counter0 > 1.0f)
		{
			addZero();
			counter0 = 0.0f;
		}
		if (counter1 > 4.0f)
		{
			addOne();
			counter1 = 0.0f;
		}
		if (counter2 > 15.0f)
		{
			addTwo();
			counter2 = 0.0f;
		}
		if (counter3 > 8.0f)
		{
			addThree();
			counter3 = 0.0f;
		}
		if (counter4 > 1.0f)
		{
			//addZero();
			counter4 = 0.0f;
		}
		if (counter5 > 3.0f)
		{
			addFive();
			counter5 = 0.0f;
		}

		std::list<Enemy*>::iterator end = mActives.end();
		std::list<Enemy*>::iterator iter = mActives.begin();
		for (;iter != end;)
		{
			Enemy* e = *iter;
			
			if (e->isAlive)
			{
				e->update(delta);
				gBulletMgr->collisionTest(e,1);
				gPlayer->collisionTest(e);
				++iter;
			}
			else
			{
				mFrees.push_back(e);
				iter = mActives.erase(iter);
				e->enableDraw(false);
			}
		}

		counter0 += delta;
		counter1 += delta;
		counter2 += delta;
		counter3 += delta;
		counter4 += delta;
		counter5 += delta;
	}

private:
	std::vector<Enemy*>	mPool;
	std::list<Enemy*>	mActives;
	std::list<Enemy*>	mFrees;
};

void initEngine(WinInfo& info)
{
	//for random
	srand((unsigned int)time(0));

	DxParam param = {info.isFullScreen,info.width,info.height,info.hwnd};
	gEngine->initAll(param,info.hwnd,info.hist,false);

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

void renderScore()
{
	GUILayout* layout = GUISystem::getSingletonPtr()->currentLayout();
	GUILabel* l0 = (GUILabel*)layout->getControlById(0);
	GUILabel* l1 = (GUILabel*)layout->getControlById(1);
	GUILabel* l2 = (GUILabel*)layout->getControlById(2);
	l0->printf("Score %d",gPoint);
	l1->printf("Alive Time %f",gLiveTime);
	char buffer[20];
	memset(buffer,0,20);
	for (int i=0; i<gPlayer->mHp; ++i)
	{
		strcat_s(buffer,"*");
	}
	l2->printf("%s",buffer);
}

void resetGame()
{
	gPoint = 0;
	gLiveTime = 0.0f;
}

void initCamera()
{
	gCamera = new Node;
	gCamera->setPosition(0.0f,50.0f,-30.0f);
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
	gEngine->getParticleSystemManager()->loadParticleSystem("media/particle/ship_gas.lua");
	gEngine->getParticleSystemManager()->loadParticleSystem("media/particle/fire.lua");
}

void releaseAll()
{
	delete gBulletMgr;
	delete gCamera;
	gBoxRenderer->release();
	delete gBoxRenderer;
	gEngine->release();
	delete gEngine;
}

void processInput(float delta)
{
	IInputSystem* input = gEngine->getInputSystem();
	if (gGameState == GAME_PAUSE)
	{
		if (input->keyDown(DIK_P))
		{
			gGameState = GAME_RUN;
		}
		return;
	}

	if (input->keyDown(DIK_P))
	{
		gGameState = GAME_PAUSE;
	}

	if (input->keyDown(DIK_F5))
	{
		gEngine->saveScreenshot();
		return;
	}

	Vector3 walk = Vector3::ZERO;
	const Vector3& cameraPos = gCamera->getPosition();

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
		gPlayer->getWeapon(0)->fire();
	}

	

	if (input->keyDown(DIK_UP) && cameraPos.y < 55.0f)
	{
		gCamera->translate(0.0f,10.0f * delta,0.0f,Node::TS_LOCAL);
	}

	if (input->keyDown(DIK_DOWN) && cameraPos.y > 45.0f)
	{
		gCamera->translate(0.0f,-10.0f * delta,0.0f,Node::TS_LOCAL);
	}

	if (input->keyDown(DIK_LEFT))
	{
		gCamera->yaw(degreeToRadians(-10 * delta));
	}

	if (input->keyDown(DIK_RIGHT))
	{
		gCamera->yaw(degreeToRadians(10 * delta));
	}

	logToScreen("ship","%f,%f",gPlayer->node->getDerivedPosition().x,gPlayer->node->getDerivedPosition().z);
	logToScreen("mouse","%f,%f",(float)input->accX(),(float)input->accY());
	//gCamera->rotate(Vector3::UNIT_Y,(float)input->accY(),Node::TS_WORLD);
	//gCamera->rotate(Vector3::UNIT_X,(float)input->accX(),Node::TS_WORLD);

	if (input->keyUp(DIK_K))
	{
		gPlayer->node->attachObject(gEngine->getParticleSystemManager()->createParticleSystem("media/particle/fire.lua"));
	}
	walk *= delta;
	Vector3 moveTo = gPlayer->node->getDerivedPosition() + walk;
	
	if (moveTo.x < -60.0f || moveTo.x > 60.0f || moveTo.z < -30.0f || moveTo.z > 200.0f)
	{
		return;
	}
	if (input->keyDown(DIK_A))
	{
		gCamera->yaw(degreeToRadians(-5 * delta));
	}
	if (input->keyDown(DIK_D))
	{
		gCamera->yaw(degreeToRadians(5 * delta));
	}
	gPlayer->node->translate(walk,Node::TS_PARENT);
}

void startGameCallback(int id)
{
	gGameState = GAME_RUN;
	GUISystem::getSingletonPtr()->changeCurrentLayout(3);
}

void backGameCallback(int id)
{
	gGameState = GAME_MENU0;
	GUISystem::getSingletonPtr()->changeCurrentLayout(0);
}

void quitGameCallback(int id)
{
	gGameState = GAME_QUIT;
}

void aboutGameCallback(int id)
{
	gGameState = GAME_MENU1;
	GUISystem* guisys = GUISystem::getSingletonPtr();
	guisys->changeCurrentLayout(1);
}

void resetGameCallback(int id)
{
	resetGame();
	gGameState = GAME_RUN;
	GUISystem::getSingletonPtr()->changeCurrentLayout(3);
}

void initGUI()
{
	GUISystem* guisys = GUISystem::getSingletonPtr();
	guisys->initOnce(*gEngine);
	loggingInit(guisys);

	guisys->load("media/gui/dx.xml");
	guisys->changeCurrentLayout(0);

	GUILayout* layout0 = guisys->getLayout(0);
	((GUIButton*)layout0->getControlById(4))->setClickListener(quitGameCallback);
	((GUIButton*)layout0->getControlById(2))->setClickListener(startGameCallback);
	((GUIButton*)layout0->getControlById(3))->setClickListener(aboutGameCallback);

	GUILayout* layout1 = guisys->getLayout(1);
	((GUIButton*)layout1->getControlById(0))->setClickListener(backGameCallback);
}

void gameRun(float delta)
{
	processInput(delta);

	switch (gGameState)
	{
	case GAME_RUN:
		gBulletMgr->update(delta);
		gEnemyMgr->update(delta);
		gPlayer->update(delta);
		renderScore();
		gLiveTime += delta;
		break;
	case GAME_MENU0:
		break;
	case GAME_MENU1:
		break;
	case GAME_MENU2:
		break;
	case GAME_MENU3:
		break;
	}
}

int WINAPI WinMain(HINSTANCE hist,HINSTANCE phist,LPSTR cmd,int show)
{
	gEngine = new DxFw;

	WinInfo info;
	if (!initWindow(800,600,false,"my game",hist,&info))
	{
		return -1;
	}

	//ShowCursor(true);

	initEngine(info);
	initEnvironment();
	initCamera();
	loadResource();

	createD3DBoundingBoxRenderer(&gBoxRenderer,gEngine->getDevice());

	DxRenderer* renderer = gEngine->getRenderer();
	IInputSystem* inputSys = gEngine->getInputSystem();
	GUISystem* guisys = GUISystem::getSingletonPtr();
	initGUI();
	
	SceneNode* root = new SceneNode("root");
	gPlayer = new MyShip(root);

	SceneNode* fire = (SceneNode*)gPlayer->node->createChild("gas");
	fire->attachObject(gEngine->getParticleSystemManager()->createParticleSystem("media/particle/ship_gas.lua"));
	fire->setPosition(0.0f,0.0f,8.0);

	gBulletMgr = new BulletManager;
	gBulletMgr->init(gEngine);
	gBulletMgr->load("media/bullet/bullets0.lua");
	root->attachObject(gBulletMgr);

	gEnemyMgr = new EnemyManager(root);

	gPlayer->addWeapon(new Weapon(0.5f,1,0.1f),0);

	gGameState = GAME_MENU0;

	MSG msg;
	while (gGameState != GAME_QUIT)
	{
		renderer->clear(true,true,true,0xff000000);
		applyCamera();

		renderer->beginScene();
		renderer->render(root);
		guisys->render();
		renderer->endScene();

		inputSys->capture();
		float delta = getTimeSinceLastFrame();
		gameRun(delta);
		if (gGameState == GAME_RUN)
		{
			root->update(delta);
			gEngine->getParticleSystemManager()->update();
		}
		Point p = inputSys->getMouseClientPosition();
		guisys->processGUI(p.x,p.y,inputSys->mouseButtonDown(0));
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