#ifndef __R_ROUTE__
#define __R_ROUTE__

#include "vector3.h"
#include "path.h"
#include <list>

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
	typedef std::list<Path*>			PathContainer;
	typedef PathContainer::iterator		PathIter;

	PathIter							mCurrentPath;
	PathContainer						mPathes;
	bool								mIsRepeat;
};

#endif