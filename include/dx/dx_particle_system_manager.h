#ifndef __DX_PARTICLE_SYSTEM_MANAGER__
#define __DX_PARTICLE_SYSTEM_MANAGER__

#include <dx/dx_dxfw.h>
#include <dx/dx_particle_system.h>
#include <map>
#include <list>
#include <string>
#include <lua.hpp>

class DxParticleSystemManager
{
public:
	static DxParticleSystemManager* getSingletonPtr()
	{
		static DxParticleSystemManager* self = new DxParticleSystemManager;
		return self;
	}

	~DxParticleSystemManager();

public:
	void				initOnce(DxFw* fw);
	void				release();

	bool				loadAllParticleSystems(const char* file);
	bool				loadParticleSystem(const char* file);
	DxParticleSystem*	createParticleSystem(const char* file);
	DxParticleAffector*	loadAffector(const char* type,const char* file);
	// destroy the unalive particle system
	void				update();
private:
	typedef std::map<std::string,lua_State*>			ParticleSystemResMap;
	typedef std::list<DxParticleSystem*>				ParticleList;
	typedef std::map<std::string,ParticleList>			ParticleSystemPool;
	typedef std::map<std::string,DxParticleAffector*>	AffectorsMap;
	typedef ParticleList::iterator						ParticleListIter;
	typedef ParticleSystemResMap::iterator				ParticleSystemResMapIter;
	typedef AffectorsMap::iterator						AffectorsMapIter;
	typedef ParticleSystemPool::iterator				ParticleSystemPoolIter;

	ParticleSystemResMap								mParticleSystemRes;
	ParticleSystemPool									mActiveParticleSystems;
	AffectorsMap										mAffectorsMap;
	DxFw*												mFw;
	ParticleSystemPool									mParticlesystemPool;

	DxParticleSystemManager();
	DxParticleSystemManager(const DxParticleSystemManager& ps);
	DxParticleSystemManager& operator=(const DxParticleSystemManager& ps);
};

#endif