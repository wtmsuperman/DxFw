#ifndef __R_PATH__
#define __R_PATH__

#include "vector3.h"

class Path
{
public:
	virtual			~Path(){}
	virtual bool	calcPosition(float delta,Vector3* out) = 0;
	virtual void	reset() = 0;
};

//////////////////////////////////////////////////////////////////////

class LinearPath : public Path
{
public:
	Vector3			startPosition;
	Vector3			endPosition;
	float			duration;
	float			currentTime;

							LinearPath(const Vector3& start,const Vector3& end,float duration);
	virtual					~LinearPath();
	inline virtual bool		calcPosition(float delta,Vector3* out);
	inline virtual void		reset();
};

bool LinearPath::calcPosition(float delta,Vector3* out)
{
	currentTime += delta;
	float fscale = clamp(currentTime / duration,0.0f,1.0f);
	*out = startPosition + (endPosition - startPosition) * fscale;
	return fscale == 1.0f;
}


void LinearPath::reset()
{
	currentTime = 0.0f;
}
///////////////////////////////////////////////////////////////////////////////////////////////

class LinearPathRel : public Path
{
public:
	Vector3		endPosition;
	Vector3		lastPosition;
	float		duration;
	float		currentTime;

							LinearPathRel(const Vector3& end,float d)
								:endPosition(end),lastPosition(0.0f,0.0f,0.0f),duration(d),currentTime(0.0f)
							{}
	virtual					~LinearPathRel(){}
	inline virtual bool		calcPosition(float delta,Vector3* out);
	inline virtual void		reset();
};

bool LinearPathRel::calcPosition(float delta,Vector3* out)
{
	currentTime += delta;
	float fscale = clamp(currentTime / duration,0.0f,1.0f);
	Vector3 currentPos = endPosition * fscale;
	*out = currentPos - lastPosition;
	lastPosition = currentPos;
	return fscale == 1.0f;
}

void LinearPathRel::reset()
{
	currentTime = 0.0f;
	lastPosition = Vector3::ZERO;
}
///////////////////////////////////////////////////////////////////////




#endif