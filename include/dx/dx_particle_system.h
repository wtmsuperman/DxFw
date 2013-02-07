#ifndef __DX_PARTICLE_SYSTEM__
#define __DX_PARTICLE_SYSTEM__

#include "dx_defines.h"
#include "dx_renderer.h"
#include "dx_vertex_structs.h"

#include <vector>
#include <queue>

struct DxParticleAttribute
{
	bool		isAlive;
	float		duration;
	float		currentTime;

	DxColor		color;
	DxColor		colorFade;

	Vector3		position;
	Vector3		velocity;
	Vector3		acceleration;
};

class DxEmitter
{
public:
	enum EmitterType
	{
		EMT_POINT,
		EMT_AABB
	};

	typedef std::vector<DxColor> ColorContaner;

	/*
	if the EmitterType is EMT_POINT,then a for postion,b is nothing;
	if the EmitterType is EMT_AABB,	then a for min,	   b for max
	*/
	Vector3			a;
	Vector3			b;
	
	Vector3			maxVelocity;
	Vector3			minVelocity;
	Vector3			maxAcceleartion;
	Vector3			minAcceleartion;

	ColorContaner	colorArray;
	DxColor			colorFade;

	float			duration;
	float			emitRate;
	EmitterType		type;

	virtual void createParticle(DxParticleAttribute* out) const;
};

class DxParticleSystem : public IRenderable
{
public:
	DxParticleSystem(IDirect3DDevice9* device,size_t maxSize);
	~DxParticleSystem();

	void release();
	void setEmitter(const DxEmitter* emitter);
	bool add();
	bool isAlive() const;

	void setBoundingBox(const AABB3& boundingbox);
	const AABB3& getBoundingBox() const {return mBoundingBox;}

	virtual void preRender(DxRenderer* renderer);
	virtual void onRender(DxRenderer* renderer);
	virtual void postRenderer(DxRenderer* renderer);

	virtual bool update(float delta);
	
private:
	typedef std::queue<size_t> FreeIndexQueue;

	IDirect3DDevice9*		mDevice;
	size_t					mMaxSize;
	DxParticleAttribute*	mAttributes;
	const DxEmitter*		mEmitter;
	float					mInvsEmitRate;
	FreeIndexQueue			mFreeIndexQueue;
	AABB3					mBoundingBox;
};

bool loadParticleSystem(DxParticleSystem* ps,DxEmitter* emitter,const char* file);

#endif