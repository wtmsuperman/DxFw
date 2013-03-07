#ifndef __DX_PARTICLE_SYSTEM__
#define __DX_PARTICLE_SYSTEM__

#include "dx_defines.h"
#include "dx_renderer.h"
#include "dx_vertex_structs.h"
#include "dx_dxfw.h"

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
	float			duration;
	float			maxTimeLL;		//max time-to-live
	float			minTimeLL;		//min time-to-live

	float			emitRate;
	float			size;

	float			maxVelocity;
	float			minVelocity;
	float			currentTime;

	DxColorValue	colorBegine;
	DxColorValue	colorEnd;

	Vector3			minPosition;
	Vector3			maxPosition;

	Vector3			direction;

	virtual void initParticle(DxParticleAttribute* p);

	virtual void genColor(DxColorValue* color);
	virtual void genDirection(Vector3* dir);
	virtual void genPosition(Vector3* pos);
	virtual void genVelocity(float* vel);
	virtual void genTimeToLive(float* time);
	virtual unsigned short  genEmissionCount(float time);
};

class DxParticleAffector
{
public:
	virtual void init(DxParticleAttribute* particle) = 0;
	virtual void affect(DxParticleAttribute* particle,float timeDelta) = 0;
};

class DxParticleAffectorFactory
{
public:
	~DxParticleAffectorFactory();

	DxParticleAffectorFactory* getSingletonPtr()
	{
		static DxParticleAffectorFactory* m = new DxParticleAffectorFactory;
		return m;
	}

	DxParticleAffector* createAffector(const char* type);

private:
	typedef std::list<DxParticleAffector*> AffectorList;

	AffectorList	mAffectors;

	DxParticleAffectorFactory(){}
	DxParticleAffectorFactory(const DxParticleAffectorFactory& v);
	DxParticleAffectorFactory& operator=(const DxParticleAffectorFactory& v);
};

class DxParticleSystem : public IRenderable
{
public:
	DxParticleSystem();
	~DxParticleSystem();

	bool init(DxFw* fw,size_t maxSize,const char* tex);
	void release();
	bool add(float delta);
	bool isAlive() const;

	void setEmitter(const DxParticleEmitter& em);

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

	typedef FreeParticleQueue::iterator		FreeParticleQueueIter;
	typedef ActiveParticleList::iterator	ActiveParticleListIter;
	typedef AffectorList::iterator			AffectorListIter;

	DxFw*					mDxFw;

	size_t					mMaxSize;
	DxParticleAttribute*	mParticlePool;
	
	FreeParticleQueue		mFreeParticles;
	ActiveParticleList		mActiveParticles;
	AffectorList			mAffectors;

	AABB3					mBoundingBox;
	DxParticleEmitter		mEmitter;

private:
	
	DxTexture*					mTex;
	IDirect3DVertexBuffer9*		mVb;
	DWORD						mVbSize;
	DWORD						mVbOffset;
	DWORD						mVbBatchSize;

private:
	DWORD	tLight; // temporary saving lighting value

};

#endif