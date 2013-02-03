#ifndef __ROUTE__
#define __ROUTE__

#include "vector3.h"
#include <vector>

class Path
{
public:
	virtual			~Path(){}
	virtual bool	calcPosition(float delta,Vector3* out) = 0;
	virtual void	reset() = 0;
};

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



class Route
{
public:
			Route(Path* path);
			Route(Path* path,bool repeat);
	virtual ~Route();

	bool calcPosition(float delta,Vector3* out);
	void addPath(Path* path);
	void reset();
	void setRepeat(bool repeat) { mIsRepeat = repeat;}
private:
	typedef std::vector<Path*>	PathContainer;
	typedef PathContainer::iterator		PathIter;

	PathIter							mCurrentPath;
	PathContainer						mPathes;
	bool								mIsRepeat;
};

#endif