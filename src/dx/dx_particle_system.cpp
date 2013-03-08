#include "dx/dx_particle_system.h"
#include "dx/dx_resource_manager.h"
#include "dx/dx_logging.h"

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

bool DxParticleSystem::update(float delta)
{
	ActiveParticleListIter end = mActiveParticles.end();
	AffectorListIter afend = mAffectors.end();
	for (ActiveParticleListIter iter=mActiveParticles.begin(); iter != end;)
	{
		DxParticleAttribute* p = *iter;
		if (p->timeToLive < delta || !mBoundingBox.cantains(p->position))
		{
			mFreeParticles.splice(mFreeParticles.end(),mActiveParticles,iter++);
		}
		else
		{
			p->position += p->velocity * delta;
			p->timeToLive -= delta;
			for (AffectorListIter ai=mAffectors.begin(); ai != afend; ++ai){
				(*ai)->affect(p,delta);
			}
			++iter;
		}
	}

	add(delta);
	logToScreen("particle","%d",mActiveParticles.size());
	return true;
}

bool DxParticleSystem::isAlive() const
{
	return !mActiveParticles.empty();
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
		//p->position;
		//p->velocity = mEmitter.direction * randf(mEmitter.minVelocity,mEmitter.maxVelocity);

		mFreeParticles.pop_front();
		
		for (AffectorListIter ai=mAffectors.begin(); ai != afend; ++ai){
			(*ai)->init(p);
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
	safe_deleteArray(mParticlePool);
	safe_delete(mEmitter);
	AffectorListIter end = mAffectors.end();
	for (AffectorListIter iter=mAffectors.begin(); iter!=end; ++iter)
	{
		safe_delete(*iter);
	}
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