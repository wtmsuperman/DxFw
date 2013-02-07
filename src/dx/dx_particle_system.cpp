#include "dx/dx_particle_system.h"

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

	switch (type)
	{
	case EMT_POINT:
		out->position = a;
		break;
	case EMT_AABB:
		out->position = randVector3(a,b);
		break;
	};
}

//
// Particle System
//

bool DxParticleSystem::update(float delta)
{
	for (size_t i=0; i<mMaxSize; ++i)
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

	static float current = 0.0f;
	current += delta;
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

void DxParticleSystem::setEmitter(const DxEmitter* emitter)
{
	this->mEmitter = emitter;
	if (emitter->emitRate <= 0.0f)
		this->mInvsEmitRate = FLT_MAX;
	else
		this->mInvsEmitRate = 1.0f / emitter->emitRate;
}

bool DxParticleSystem::add()
{
	if (mFreeIndexQueue.empty())
		return false;
	mEmitter->createParticle(&mAttributes[mFreeIndexQueue.front()]);
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