#include "dx/dx_particle_system.h"
#include "dx/dx_resource_manager.h"
#include "dx/dx_logging.h"
#include <dx/dx_particle_system_manager.h>
#include <node/scene_node.h>

//
// Particle Emitter
//

DxParticleEmitter::DxParticleEmitter()
{
	DxColorValue t = {1.0f,1.0f,1.0f,1.0f};
	colorBegine = t;
	colorEnd = t;
	angle = 0.0f;
	up = Vector3::UNIT_Y;
	direction = Vector3::UNIT_Z;
	duration = 0.0f;
	emitRate = 1000.0f;
	maxPosition = Vector3(0.0f,0.0f,0.0f);
	minPosition = Vector3(0.0f,0.0f,0.0f);
	minVelocity = 1.0f;
	maxVelocity = 10.0f;
	maxTimeLL = 1.0f;
	minTimeLL = 1.0f;
	size = 1.0f;
	repeatDelay = 0.0f;
	currentTime = duration;
	repeatTimeRemain = repeatDelay;
	enable = true;
}

void DxParticleEmitter::reset()
{
	enable = true;
	currentTime = duration;
	repeatTimeRemain = repeatDelay;
}

void DxParticleEmitter::initParticle(DxParticleAttribute* p)
{
	genColor(&p->color);
	genPosition(&p->position);
	genTimeToLive(&p->timeToLive);
	float vel;
	Vector3 dir;
	genVelocity(&vel);
	genDirection(&dir);
	p->velocity = dir * vel;
}

void DxParticleEmitter::genColor(DxColorValue* color)
{
	color->a = randf(colorBegine.a,colorEnd.a);
	color->b = randf(colorBegine.b,colorEnd.b);
	color->g = randf(colorBegine.g,colorEnd.g);
	color->r = randf(colorBegine.r,colorEnd.r);
}

void DxParticleEmitter::genDirection(Vector3* dir)
{
	 if (angle != 0.0f)
     {
            // Randomise angle
            float tangle = randf() * angle;

            // Randomise direction
            *dir = direction.randomDeviant(degreeToRadians(tangle), up);
        }
        else
        {
            // Constant angle
            *dir = direction;
        }
}

void DxParticleEmitter::genPosition(Vector3* pos)
{
	*pos = randVector3(minPosition,maxPosition);
}

void DxParticleEmitter::genVelocity(float* vel)
{
	*vel = randf(minVelocity,maxVelocity);
}

void DxParticleEmitter::genTimeToLive(float* time)
{
	*time = randf(minTimeLL,maxTimeLL);
}

unsigned short DxParticleEmitter::genEmissionCount(float time)
{
	if (enable)
	{
		static float remainder = 0.0f;
		remainder += emitRate * time;
        unsigned short intRequest = (unsigned short)remainder;
        remainder -= intRequest;

		if (duration != 0.0f)
		{
			currentTime -= time;
			if (currentTime <= 0.0f)
			{
				enable = false;
			}
		}
		return intRequest;
	}
	else
	{
		if (repeatDelay != 0.0f)
		{
			repeatTimeRemain -= time;
			if (repeatTimeRemain <= 0.0f)
			{
				enable = true;
				repeatTimeRemain = repeatDelay;
				currentTime = duration;
			}
		}
	}
	return 0;
}

//
// Particle System
//

DxParticleSystem::DxParticleSystem()
{
	mDxFw = 0;
	mTex = 0;
	mVb = 0;

	mVbSize = 2048;
	mVbOffset = 0;
	mVbBatchSize = 512;
	mBoundingBox.makeInfinity();
}

bool DxParticleSystem::init(DxFw* fw,size_t maxSize,const char* tex)
{
	mParticlePool = new DxParticleAttribute[maxSize];
	if (mParticlePool == 0)
		return false;

	mDxFw = fw;
	mMaxSize = maxSize;

	for (size_t i=0;i < maxSize; ++i){
		mFreeParticles.push_back(&mParticlePool[i]);
	}

	if (FAILED(fw->getDevice()->CreateVertexBuffer(mVbSize * sizeof(ParticleVertex)
		,D3DUSAGE_DYNAMIC | D3DUSAGE_POINTS | D3DUSAGE_WRITEONLY
		,ParticleVertex::FVF
		,D3DPOOL_DEFAULT
		,&mVb
		,0)))
	{
		return false;
	}

	mTex = mDxFw->getResourceManager()->loadTexture(DEFAULT_GROUP_NAME,tex);

	return true;
}

DxParticleSystem::~DxParticleSystem()
{
	release();
}

void DxParticleSystem::update(float delta)
{
	ActiveParticleListIter end = mActiveParticles.end();
	
	for (ActiveParticleListIter iter=mActiveParticles.begin(); iter != end;)
	{
		DxParticleAttribute* p = *iter;
		if (p->timeToLive < delta || !mBoundingBox.contains(p->position))
		{
			mFreeParticles.splice(mFreeParticles.end(),mActiveParticles,iter++);
		}
		else
		{
			p->position += p->velocity * delta;
			p->timeToLive -= delta;
			++iter;
		}
	}

	add(delta);
	affect(delta);
	logToScreen("particle","%d",mActiveParticles.size());
}

void DxParticleSystem::affect(float delta)
{
	AffectorListIter afend = mAffectors.end();
	for (AffectorListIter ai=mAffectors.begin(); ai != afend; ++ai){
			(*ai)->affect(this,delta);
	}
}

bool DxParticleSystem::isAlive() const
{
	if (mEmitter->enable || mEmitter->repeatDelay != 0.0f)
	{
		return true;
	}
	else if (!mActiveParticles.empty())
	{
		return true;
	}
	else
		return false;
}

void DxParticleSystem::setEmitter(DxParticleEmitter* emitter)
{
	this->mEmitter = emitter;
}

void DxParticleSystem::addAffector(DxParticleAffector* affector)
{
	this->mAffectors.push_back(affector);
}

bool DxParticleSystem::add(float timeDelta)
{
	unsigned short r =  mEmitter->genEmissionCount(timeDelta);
	//logToScreen("r","%d",r);
	if (r == 0)
		return false;

	float timePoint = 0.0f;
	float timeInc = timeDelta / r;

	AffectorListIter afend = mAffectors.end();
	while ( r > 0 && !mFreeParticles.empty())
	{
		DxParticleAttribute* p = mFreeParticles.front();
		
		mEmitter->initParticle(p);

		//这里需要更改
		// transform to world space
		if (mParent)
		{
			p->position = mParent->getDerivedPosition() + p->position;
			Matrix3x3 rotation(mParent->getDerivedOrientation());
			p->velocity *= rotation;
		}
		//p->velocity = mEmitter.direction * randf(mEmitter.minVelocity,mEmitter.maxVelocity);

		mFreeParticles.pop_front();
		
		for (AffectorListIter ai=mAffectors.begin(); ai != afend; ++ai){
			(*ai)->init(this);
		}

		p->position += p->velocity * timePoint;
		timePoint += timeInc;

		mActiveParticles.push_back(p);
		r--;
	}
	return true;
}

void DxParticleSystem::release()
{
	safe_Release(mVb);
	safe_deleteArray(mParticlePool);
	safe_delete(mEmitter);
}

void DxParticleSystem::setBoundingBox(const AABB3& box)
{
	mBoundingBox = box;
}

void DxParticleSystem::setVertexBufferAttribute(DWORD vbsize,DWORD batchSize)
{
	mVbSize = vbsize;
	mVbBatchSize = batchSize;
}

void DxParticleSystem::preRender(DxRenderer* renderer)
{
	renderer->getDevice()->GetRenderState(D3DRS_LIGHTING,&tLight);
	renderer->setRenderState(D3DRS_LIGHTING,false);
	renderer->setRenderState(D3DRS_ZWRITEENABLE,false);
	renderer->setRenderState(D3DRS_POINTSPRITEENABLE,true);
	renderer->setRenderState(D3DRS_POINTSCALEENABLE,true);
	renderer->setRenderState(D3DRS_POINTSIZE,FtoDW(mEmitter->size));
	renderer->setRenderState(D3DRS_POINTSIZE_MIN,FtoDW(0.f));

	renderer->setRenderState(D3DRS_POINTSCALE_A,FtoDW(0.0f));
	renderer->setRenderState(D3DRS_POINTSCALE_B,FtoDW(0.0f));
	renderer->setRenderState(D3DRS_POINTSCALE_C,FtoDW(1.0f));

	renderer->setTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	renderer->setTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
	renderer->enableTransparent();
}

void DxParticleSystem::postRender(DxRenderer* renderer)
{
	renderer->setRenderState(D3DRS_ZWRITEENABLE,true);
	renderer->setRenderState(D3DRS_POINTSPRITEENABLE,false);
	renderer->setRenderState(D3DRS_POINTSCALEENABLE,false);
	renderer->setRenderState(D3DRS_LIGHTING,tLight);
	renderer->disableTransparent();
}

void DxParticleSystem::onRender(DxRenderer* renderer)
{
	
	if (!mActiveParticles.empty())
	{
		renderer->applyTexture(0,mTex);
		renderer->setWorldTransform(Matrix4x4::IDENTITY);

		IDirect3DDevice9* device = renderer->getDevice();

		device->SetFVF(ParticleVertex::FVF);
		device->SetStreamSource(0,mVb,0,sizeof(ParticleVertex));

		if (mVbOffset >= mVbSize){
			mVbOffset = 0;
		}

		ParticleVertex* v = 0;
		mVb->Lock(mVbOffset * sizeof(ParticleVertex)
			,mVbBatchSize * sizeof(ParticleVertex)
			,(void**)&v
			,mVbOffset ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);

		DWORD numParticleInBatch = 0;
		ActiveParticleListIter end = mActiveParticles.end();
		for (ActiveParticleListIter iter=mActiveParticles.begin(); iter != end; ++iter){
			DxParticleAttribute* p = *iter;
				v->x = p->position.x;
				v->y = p->position.y;
				v->z = p->position.z;
				v->color = D3DCOLOR_COLORVALUE(p->color.r,p->color.g,p->color.b,p->color.r);
				++v;
				++numParticleInBatch;

				if (numParticleInBatch == mVbBatchSize){
					mVb->Unlock();
					device->DrawPrimitive(D3DPT_POINTLIST,mVbOffset,mVbBatchSize);

					mVbOffset += mVbBatchSize;
					if (mVbOffset >= mVbSize){
						mVbOffset = 0;
					}

					mVb->Lock(mVbOffset*sizeof(ParticleVertex),mVbBatchSize*sizeof(ParticleVertex),(void**)&v,mVbOffset ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);
					numParticleInBatch = 0;
				}
		}

		mVb->Unlock();
		if (numParticleInBatch != 0){
			device->DrawPrimitive(D3DPT_POINTLIST,mVbOffset,numParticleInBatch);
		}
		mVbOffset += mVbBatchSize;
	}
}

void DxParticleSystem::reset()
{
	mEmitter->reset();
	while (!mActiveParticles.empty())
	{
		mFreeParticles.push_back(mActiveParticles.front());
		mActiveParticles.pop_front();
	}
}

bool loadParticleSystem(DxParticleSystem* o,DxFw* fw,lua_State* L)
{
	lua_getglobal(L,"particle_system");
	
	lua_getfield(L,1,"max_size");
	lua_getfield(L,1,"texture");
	lua_getfield(L,1,"vertex_buffer_size");
	lua_getfield(L,1,"batch_size");

	if (lua_isnumber(L,2))
	{
		if (lua_isstring(L,3))
		{
			o->init(fw,(size_t)lua_tonumber(L,2),lua_tostring(L,3));
		}
		else
		{
			o->init(fw,(size_t)lua_tonumber(L,2),"");
		}
	}
	else
	{
		lua_close(L);
		return false;
	}

	if (lua_isnumber(L,4) && lua_isnumber(L,5))
	{
		o->setVertexBufferAttribute((DWORD)lua_tonumber(L,2),(DWORD)lua_tonumber(L,3));
	}

	lua_pop(L,1); // pop batch_size
	lua_pop(L,1); // pop vertex_buffer_size
	lua_pop(L,1); // pop texture;
	lua_pop(L,1); // pop max_size;
	lua_pop(L,1); // pop particle_system

	////////////////////////////////////////////////////////////////////
	
	//emitter
	lua_getglobal(L,"emitter");
	static char items[][32] = { 
		"position_max"
		,"position_min"
		,"velocity_max"
		,"velocity_min"
		,"color_begine"
		,"color_end"
		,"time_to_live_max"
		,"time_to_live_min"
		,"duration"
		,"repeat_delay"
		,"angle"
		,"size"
		,"emit_rate"
		,"direction"
	};

	for (int i=0; i<14; ++i)
	{
		lua_getfield(L,1,items[i]);
	}

	DxParticleEmitter* emitter = new DxParticleEmitter;

	//direction
	if (lua_istable(L,-1))
	{
		lua_rawgeti(L,-1,1); //x
		lua_rawgeti(L,-2,2); //y
		lua_rawgeti(L,-3,3); //z
		emitter->direction = Vector3((float)lua_tonumber(L,-3),(float)lua_tonumber(L,-2),(float)lua_tonumber(L,-1));
		lua_pop(L,1); // pop z
		lua_pop(L,1); // pop y
		lua_pop(L,1); // pop x
	}
	lua_pop(L,1); // pop direction

	//emit_rate
	if (lua_isnumber(L,-1))
	{
		emitter->emitRate = (float)lua_tonumber(L,-1);
	}
	lua_pop(L,1); // pop emit_rate

	//emit_size
	if (lua_isnumber(L,-1))
	{
		emitter->size = (float)lua_tonumber(L,-1);
	}
	lua_pop(L,1); // pop size

	//angle
	if (lua_isnumber(L,-1))
	{
		emitter->angle = (float)lua_tonumber(L,-1);
	}
	lua_pop(L,1); // pop angle

	//repeat_delay
	if (lua_isnumber(L,-1))
	{
		emitter->repeatDelay = (float)lua_tonumber(L,-1);
	}
	lua_pop(L,1); // pop repeat_delay

	//duration
	if (lua_isnumber(L,-1))
	{
		emitter->duration = (float)lua_tonumber(L,-1);
	}
	lua_pop(L,1); // pop duration

	//time_to_live_min
	if (lua_isnumber(L,-1))
	{
		emitter->minTimeLL = (float)lua_tonumber(L,-1);
	}
	lua_pop(L,1); // pop time_to_live_min

	//time_to_live_max
	if (lua_isnumber(L,-1))
	{
		emitter->maxTimeLL = (float)lua_tonumber(L,-1);
	}
	lua_pop(L,1); // pop time_to_live_max

	//color_end
	if (lua_istable(L,-1))
	{
		lua_rawgeti(L,-1,1); //a
		lua_rawgeti(L,-2,2); //r
		lua_rawgeti(L,-3,3); //g
		lua_rawgeti(L,-4,4); //b
		emitter->colorEnd.a = (float)lua_tonumber(L,-4);
		emitter->colorEnd.r = (float)lua_tonumber(L,-3);
		emitter->colorEnd.g = (float)lua_tonumber(L,-2);
		emitter->colorEnd.b = (float)lua_tonumber(L,-1);
		lua_pop(L,1); // pop b
		lua_pop(L,1); // pop g
		lua_pop(L,1); // pop r
		lua_pop(L,1); // pop a
	}
	lua_pop(L,1); // pop color_end

	//color_begine
	if (lua_istable(L,-1))
	{
		lua_rawgeti(L,-1,1); //a
		lua_rawgeti(L,-2,2); //r
		lua_rawgeti(L,-3,3); //g
		lua_rawgeti(L,-4,4); //b
		emitter->colorBegine.a = (float)lua_tonumber(L,-4);
		emitter->colorBegine.r = (float)lua_tonumber(L,-3);
		emitter->colorBegine.g = (float)lua_tonumber(L,-2);
		emitter->colorBegine.b = (float)lua_tonumber(L,-1);
		lua_pop(L,1); // pop b
		lua_pop(L,1); // pop g
		lua_pop(L,1); // pop r
		lua_pop(L,1); // pop a
	}
	lua_pop(L,1); // pop color_begine

	//velocity_min
	if (lua_isnumber(L,-1))
	{
		emitter->minVelocity = (float)lua_tonumber(L,-1);
	}
	lua_pop(L,1); // pop velocity_min

	//velocity_max
	if (lua_isnumber(L,-1))
	{
		emitter->maxVelocity = (float)lua_tonumber(L,-1);
	}
	lua_pop(L,1); // pop velocity_max

	//position_min
	if (lua_istable(L,-1))
	{
		lua_rawgeti(L,-1,1); //x
		lua_rawgeti(L,-2,2); //y
		lua_rawgeti(L,-3,3); //z
		emitter->minPosition = Vector3((float)lua_tonumber(L,-3),(float)lua_tonumber(L,-2),(float)lua_tonumber(L,-1));
		lua_pop(L,1); // pop z
		lua_pop(L,1); // pop y
		lua_pop(L,1); // pop x
	}
	lua_pop(L,1); // pop position_min

	//position_max
	if (lua_istable(L,-1))
	{
		lua_rawgeti(L,-1,1); //x
		lua_rawgeti(L,-2,2); //y
		lua_rawgeti(L,-3,3); //z
		emitter->maxPosition = Vector3((float)lua_tonumber(L,-3),(float)lua_tonumber(L,-2),(float)lua_tonumber(L,-1));
		lua_pop(L,1); // pop z
		lua_pop(L,1); // pop y
		lua_pop(L,1); // pop x
	}
	lua_pop(L,1); // pop position_max

	lua_pop(L,1); // pop emitter

	o->setEmitter(emitter);

	lua_getglobal(L,"affectors");

	lua_pushnil(L);
	while (lua_next(L,1) != 0)
	{
		// key at -2,value at -1
		o->addAffector(DxParticleSystemManager::getSingletonPtr()->loadAffector(lua_tostring(L,-2),lua_tostring(L,-1)));
		lua_pop(L,1);
	}

	lua_pop(L,1); //pop affectors

	return true;
}