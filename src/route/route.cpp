#include "route/route.h"

LinearPath::LinearPath(const Vector3& start,const Vector3& end,float d)
	:startPosition(start),endPosition(end),duration(d),currentTime(0.0f)
{}

LinearPath::~LinearPath()
{}


//////////////////////////////////////////////////////////////////////////////


//
// Route
//

Route::Route(Path* path)
{
	mPathes.push_back(path);
	mCurrentPath = mPathes.begin();
	mIsRepeat = false;
}

Route::Route(Path* path,bool r)
{
	mPathes.push_back(path);
	mCurrentPath = mPathes.begin();
	mIsRepeat = r;
}

Route::~Route()
{
	PathIter end = mPathes.end();
	for (PathIter iter = mPathes.begin(); iter != end; ++iter)
	{
		delete *iter;
	}
	mPathes.clear();
}

void Route::addPath(Path* path)
{
	mPathes.push_back(path);
}

bool Route::calcPosition(float delta,Vector3* out)
{
	if (mCurrentPath == mPathes.end())
		return true;

	if ((*mCurrentPath)->calcPosition(delta,out))
	{
		mCurrentPath++;
		if (mCurrentPath == mPathes.end())
			return true;
	}

	return false;
}

void Route::reset()
{
	PathIter end = mPathes.end();
	for (PathIter iter = mPathes.begin(); iter != end; ++iter)
	{
		(*iter)->reset();
	}
	mCurrentPath = mPathes.begin();
}