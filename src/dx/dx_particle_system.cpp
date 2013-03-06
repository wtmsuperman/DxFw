#include "dx/dx_particle_system.h"
#include "dx/dx_resource_manager.h"

void DxEmitter::createParticle(DxParticleAttribute* out) const
{
	out->velocity = randVector3(minVelocity,maxVelocity);
	out->acceleration = randVector3(minAcceleartion,maxAcceleartion);
	out->colorFade = colorFade;
	out->duration = duration;
	out->isAlive = true;
	out->currentTime = 0.0f;

	size_t size = colorArray.size();
	size_t i = randl(0,size);
	out->color = colorArray[i];

	out->position = randVector3(minPosition,maxPosition);
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
	mAttributes = new DxParticleAttribute[maxSize];
	if (mAttributes == 0)
		return false;

	mDxFw = fw;
	mMaxSize = maxSize;

	for (size_t i=0; i<maxSize; ++i)
	{
		mFreeIndexQueue.push(i);
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
	safe_Release(mVb);
	if (mTex)
		mDxFw->getResourceManager()->releaseTexture("particle",mTex->name);
	safe_deleteArray(mAttributes);
}

bool DxParticleSystem::update(float delta)
{
	static float current = 0.0f;
	size_t size = mMaxSize - mFreeIndexQueue.size();
	for (size_t i=0; i<size; ++i)
	{
		DxParticleAttribute& p = mAttributes[i];
		if (p.isAlive)
		{
			p.position += p.velocity * delta;
			p.velocity += p.acceleration * delta;
			p.currentTime += delta;
			p.color.r += p.colorFade.r * delta;
			p.color.g += p.colorFade.g * delta;
			p.color.b += p.colorFade.b * delta;
			p.color.a += p.colorFade.a * delta;
			if (p.currentTime >= p.duration || !mBoundingBox.cantains(p.position))
			{
				p.isAlive = false;
				mFreeIndexQueue.push(i);
			}
		}
	}

	current += delta;
	//add();
	if (current >= mInvsEmitRate)
	{
		current = 0.0f;
		add();
	}

	return true;
}

bool DxParticleSystem::isAlive() const
{
	return mFreeIndexQueue.size() == mMaxSize;
}

size_t DxParticleSystem::size()
{
	return mMaxSize - mFreeIndexQueue.size();
}

void DxParticleSystem::setEmitter(const DxEmitter* emitter)
{
	this->mEmitter = *emitter;
	this->mInvsEmitRate = 1.0f / (emitter->emitRate * 1000.0f);
}

bool DxParticleSystem::add()
{
	if (mFreeIndexQueue.empty())
		return false;
	mEmitter.createParticle(&mAttributes[mFreeIndexQueue.front()]);
	mFreeIndexQueue.pop();
	return true;
}

void DxParticleSystem::release()
{
	safe_deleteArray(mAttributes);
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
	renderer->setRenderState(D3DRS_POINTSPRITEENABLE,true);
	renderer->setRenderState(D3DRS_POINTSCALEENABLE,true);
	renderer->setRenderState(D3DRS_POINTSIZE,FtoDW(mEmitter.size));
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
	renderer->setRenderState(D3DRS_POINTSPRITEENABLE,false);
	renderer->setRenderState(D3DRS_POINTSCALEENABLE,false);
	renderer->setRenderState(D3DRS_LIGHTING,tLight);
	renderer->disableTransparent();
}

void DxParticleSystem::onRender(DxRenderer* renderer)
{
	size_t size = mMaxSize - mFreeIndexQueue.size();
	if (size > 0)
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
		for (size_t i=0; i<size; ++i){
			DxParticleAttribute* p = &mAttributes[i];
			if (p->isAlive){
				v->x = p->position.x;
				v->y = p->position.y;
				v->z = p->position.z;
				v->color = (D3DCOLOR)(p->color);
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
		}

		mVb->Unlock();
		if (numParticleInBatch != 0){
			device->DrawPrimitive(D3DPT_POINTLIST,mVbOffset,numParticleInBatch);
		}
		mVbOffset += mVbBatchSize;
	}
}