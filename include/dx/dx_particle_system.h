#ifndef __DX_PARTICLE_SYSTEM__
#define __DX_PARTICLE_SYSTEM__

#include "dx_defines.h"
#include "dx_renderer.h"
#include "dx_vertex_structs.h"
#include "dx_dxfw.h"

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
	typedef std::vector<DxColor> ColorContaner;

	Vector3			minPosition;
	Vector3			maxPosition;
	Vector3			maxVelocity;
	Vector3			minVelocity;
	Vector3			maxAcceleartion;
	Vector3			minAcceleartion;

	ColorContaner	colorArray;
	DxColor			colorFade;

	float			duration;
	float			emitRate;
	float			size;

	virtual void createParticle(DxParticleAttribute* out) const;
};

class DxParticleSystem : public IRenderable
{
public:
	DxParticleSystem();
	~DxParticleSystem();

	bool init(DxFw* fw,size_t maxSize,const char* tex);
	void release();
	void setEmitter(const DxEmitter* emitter);
	bool add();
	bool isAlive() const;

	void setVertexBufferAttribute(DWORD vbsize,DWORD batchSize);

	void setBoundingBox(const AABB3& boundingbox);
	const AABB3& getBoundingBox() const {return mBoundingBox;}

	size_t size();

	virtual void preRender(DxRenderer* renderer);
	virtual void onRender(DxRenderer* renderer);
	virtual void postRender(DxRenderer* renderer);

	virtual bool update(float delta);
	
private:
	typedef std::queue<size_t> FreeIndexQueue;
	
	DxFw*					mDxFw;
	size_t					mMaxSize;
	DxParticleAttribute*	mAttributes;
	DxEmitter				mEmitter;
	float					mInvsEmitRate;
	FreeIndexQueue			mFreeIndexQueue;
	AABB3					mBoundingBox;

private:
	
	DxTexture*					mTex;
	IDirect3DVertexBuffer9*		mVb;
	DWORD						mVbSize;
	DWORD						mVbOffset;
	DWORD						mVbBatchSize;

private:
	DWORD	tLight; // temporary saving lighting value

};

bool loadParticleSystem(DxParticleSystem* ps,DxEmitter* emitter,const char* file);

#endif