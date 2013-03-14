#include <dx/dx_particle_system_manager.h>

DxParticleSystemManager::DxParticleSystemManager()
{
	registAllDefaultAffectos();
}

DxParticleSystemManager::~DxParticleSystemManager()
{
	release();
}

void DxParticleSystemManager::initOnce(DxFw* fw)
{
	static bool f = true;
	if (f)
	{
		f = false;
		this->mFw = fw;
	}
}

void DxParticleSystemManager::release()
{
	ParticleSystemPoolIter poolEnd = mParticlesystemPool.end();
	for (ParticleSystemPoolIter piter=mParticlesystemPool.begin(); piter!=poolEnd; ++piter)
	{
		ParticleList& list = piter->second;
		ParticleListIter end = list.end();
		for (ParticleListIter iter=list.begin(); iter!=end; ++iter)
		{
			DxParticleSystem* ps = *iter;
			delete ps;
		}
	}
	mParticlesystemPool.clear();

	ParticleSystemResMapIter resEnd = mParticleSystemRes.end();
	for (ParticleSystemResMapIter iter=mParticleSystemRes.begin(); iter!=resEnd; ++iter)
	{
		lua_close(iter->second);
	}

	mParticleSystemRes.clear();

	AffectorsMapIter affectorEnd = mAffectorsMap.end();
	for  (AffectorsMapIter iter=mAffectorsMap.begin(); iter != affectorEnd; ++iter)
	{
		delete iter->second;
	}
	mAffectorsMap.clear();
}

void DxParticleSystemManager::update()
{
	ParticleSystemPoolIter poolEnd = mActiveParticleSystems.end();
	for (ParticleSystemPoolIter piter=mActiveParticleSystems.begin(); piter!=poolEnd; ++piter)
	{
		ParticleList& list = piter->second;
		ParticleListIter end = list.end();
		for (ParticleListIter iter=list.begin(); iter!=end;)
		{
			DxParticleSystem* ps = *iter;
			if (!ps->isAlive())
			{
				iter = list.erase(iter);
				mParticlesystemPool[piter->first].push_back(ps);
				ps->getParent()->detachObject(ps);
			//delete ps;
			}
			else
			{
				++iter;
			}
		}
	}
}

bool DxParticleSystemManager::loadParticleSystem(const char* file)
{
	//池里面有，说明已经载入过
	ParticleSystemPoolIter poolIter = mParticlesystemPool.find(file);
	if (poolIter != mParticlesystemPool.end())
	{
		return true;
	}

	ParticleSystemResMapIter iter = mParticleSystemRes.find(file);
	if (iter != mParticleSystemRes.end())
	{
		return true;
	}

	//insert a new particle system script
	lua_State* L = lua_open();
	if (luaL_loadfile(L,file))
	{
		assert(true && "particle system script path error");
		lua_close(L);
		return false;
	}
	lua_pcall(L,0,0,0);

	mParticleSystemRes[file] = L;

	ParticleList& list = mParticlesystemPool[file];
	//插入5个缓存
	for (int i=0; i < 5; ++i)
	{
		DxParticleSystem* ps = new DxParticleSystem;
		::loadParticleSystem(ps,mFw,L);
		list.push_back(ps);
	}

	return true;
}

DxParticleSystem* DxParticleSystemManager::createParticleSystem(const char* file)
{
	ParticleSystemResMapIter iter = mParticleSystemRes.find(file);
	if (iter == mParticleSystemRes.end())
	{
		assert(false && "no such particle system");
		return 0;
	}

	//资源中有，必然不为空
	ParticleList& list = mParticlesystemPool[file];

	//缓存池用尽
	if (list.empty())
	{
		DxParticleSystem* ps = new DxParticleSystem;
		::loadParticleSystem(ps,mFw,iter->second);
		mActiveParticleSystems[file].push_back(ps);
		return ps;
	}
	else
	{
		DxParticleSystem* ps = list.front();
		ps->reset();
		mActiveParticleSystems[file].push_back(ps);
		list.pop_front();
		return ps;
	}
}

DxParticleAffector* DxParticleSystemManager::loadAffector(const char* type,const char* file)
{
	char buffer[512];
	sprintf_s(buffer,("%s_%s"),type,file);
	AffectorsMapIter iter = mAffectorsMap.find(buffer);
	if (iter != mAffectorsMap.end())
	{
		return iter->second;
	}
	else
	{
		AffectorCreator creator = getAffectorCreator(type);
		if (creator == 0)
		{
			assert(false && "load affector failed");
			return 0;
		}
		else
		{
			DxParticleAffector* affector =  creator(file);
			mAffectorsMap[buffer] = affector;
			return affector;
		}
	}
}