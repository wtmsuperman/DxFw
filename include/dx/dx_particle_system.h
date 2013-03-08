#ifndef __DX_PARTICLE_SYSTEM__
#define __DX_PARTICLE_SYSTEM__

#include "dx_defines.h"
#include "dx_renderer.h"
#include "dx_vertex_structs.h"
#include "dx_dxfw.h"
#include "dx_particle_system_affector.h"
#include <node/attachable.h>

#include <list>

class DxParticleAttribute
{
public:
	float			timeToLive;
	DxColorValue	color;
	Vector3			position;
	Vector3			velocity;
};

class DxParticleEmitter
{
public:
	bool			enable;
	bool			isRepeat;

	// zero for forever
	float			duration; 
	// zero for no repeat
	float			repeatDelay; 
	float			currentTime;
	float			repeatTimeRemain;

	//max time-to-live
	float			maxTimeLL;
	//min time-to-live
	float			minTimeLL;		

	float			emitRate;
	float			size;

	float			angle;

	float			maxVelocity;
	float			minVelocity;

	DxColorValue	colorBegine;
	DxColorValue	colorEnd;

	Vector3			minPosition;
	Vector3			maxPosition;

	Vector3			direction;
	// Notional up vector, used to speed up 
	// generation of variant directions, and also to orient some emitters.
	Vector3			up; 

	// a reasonable initialize
	DxParticleEmitter();

	virtual void initParticle(DxParticleAttribute* p);

	virtual void genColor(DxColorValue* color);
	virtual void genDirection(Vector3* dir);
	virtual void genPosition(Vector3* pos);
	virtual void genVelocity(float* vel);
	virtual void genTimeToLive(float* time);
	virtual unsigned short  genEmissionCount(float time);
};

class DxParticleSystem : public AttachableObject
{
public:
	DxParticleSystem();
	~DxParticleSystem();

	bool init(DxFw* fw,size_t maxSize,const char* tex);
	void release();
	bool add(float delta);
	void affect(float delta);
	bool isAlive() const;

	//note that emitter will be release by current
	//particle system release() function
	void setEmitter(DxParticleEmitter* em);

	//note that affector will be released by current 
	//particle system release() function
	void addAffector(DxParticleAffector* affector);

	void setVertexBufferAttribute(DWORD vbsize,DWORD batchSize);

	void setBoundingBox(const AABB3& boundingbox);

	const AABB3&		getBoundingBox() const {return mBoundingBox;}

	virtual void preRender(DxRenderer* renderer);
	virtual void onRender(DxRenderer* renderer);
	virtual void postRender(DxRenderer* renderer);

	virtual bool update(float delta);
	
private:
	typedef std::list<DxParticleAttribute*> FreeParticleQueue;
	typedef std::list<DxParticleAttribute*> ActiveParticleList;
	typedef std::list<DxParticleAffector*>	AffectorList;

public:
	typedef FreeParticleQueue::iterator		FreeParticleQueueIter;
	typedef ActiveParticleList::iterator	ActiveParticleListIter;
	typedef AffectorList::iterator			AffectorListIter;

private:
	DxFw*					mDxFw;

	size_t					mMaxSize;
	DxParticleAttribute*	mParticlePool;
	
	FreeParticleQueue		mFreeParticles;
	ActiveParticleList		mActiveParticles;
	AffectorList			mAffectors;

	AABB3					mBoundingBox;
	DxParticleEmitter*		mEmitter;

public:
	ActiveParticleListIter	activeParticleBegin()
	{
		return mActiveParticles.begin();
	}
	ActiveParticleListIter	activeParticleEnd()
	{
		return mActiveParticles.end();
	}

private:
	
	DxTexture*					mTex;
	IDirect3DVertexBuffer9*		mVb;
	DWORD						mVbSize;
	DWORD						mVbOffset;
	DWORD						mVbBatchSize;
private:
	DWORD	tLight; // temporary saving lighting value
};

void loadParticleSystem(DxParticleSystem* o,const char* fileName);

#endif