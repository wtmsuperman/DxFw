#include <dx/dx_particle_system_manager.h>

void DxParticleSystemManager::release()
{
	ParticleListIter end = mActiveParticleSystems.end();
	for (ParticleListIter iter=mActiveParticleSystems.begin(); iter!=end; ++iter)
	{
		delete (*iter);
	}

	mParticleSystemRes.clear();
}

void DxParticleSystemManager::update()
{
	ParticleListIter end = mActiveParticleSystems.end();
	for (ParticleListIter iter=mActiveParticleSystems.begin(); iter!=end;)
	{
		DxParticleSystem* ps = *iter;
		if (ps->isAlive())
		{
			iter = mActiveParticleSystems.erase(iter);
			delete ps;
		}
		else
		{
			++iter;
		}
	}

	mParticleSystemRes.clear();
}

bool DxParticleSystemManager::loadParticleSystem(const char* file)
{
	if (mParticleSystemRes.count(file) != 0)
	{
		return true;
	}

	//insert a new particle system
	DxParticleSystem& ps = mParticleSystemRes[file];
	return ::loadParticleSystem(&ps,mFw,file);
}

DxParticleSystem* DxParticleSystemManager::createParticleSystem(const char* file)
{
	ParticleSystemResMapIter iter = mParticleSystemRes.find(file);
	if (iter == mParticleSystemRes.end())
	{
		assert(false && "no such particle system");
		return 0;
	}

	DxParticleSystem* ps = new DxParticleSystem;
	*ps = mParticleSystemRes[file];
	mActiveParticleSystems.push_back(ps);
	return ps;
}