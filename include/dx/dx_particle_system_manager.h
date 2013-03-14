#ifndef __DX_PARTICLE_SYSTEM_MANAGER__
#define __DX_PARTICLE_SYSTEM_MANAGER__

#include <dx/dx_dxfw.h>
#include <dx/dx_particle_system.h>
#include <map>
#include <list>

class DxParticleSystemManager
{
public:
	void				release();

	bool				loadAllParticleSystems(const char* file);
	bool				loadParticleSystem(const char* file);
	DxParticleSystem*	createParticleSystem(const char* file);
	DxParticleAffector*	loadAffector(const char* file);
	// destroy the unalive particle system
	void				update();
private:
	//������Ҫ���ƣ�û�б�ҪΪ�˽�ʡһ�θ��ƶ�����ָ��
	typedef std::map<std::string,DxParticleSystem>		ParticleSystemResMap;
	typedef std::list<DxParticleSystem*>				ParticleList;
	typedef std::map<std::string,DxParticleAffector*>	AffectorsMap;
	typedef ParticleList::iterator						ParticleListIter;
	typedef ParticleSystemResMap::iterator				ParticleSystemResMapIter;

	ParticleSystemResMap							mParticleSystemRes;
	ParticleList									mActiveParticleSystems;
	DxFw*											mFw;
};

#endif