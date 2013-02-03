#include "route/route.h"

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

	if ((*mCurrentPath)->calcPosition(delta,out)){
		mCurrentPath++;
		if (mCurrentPath == mPathes.end()){
			if (mIsRepeat){
				reset();
				return false;
			}else{
				return true;
			}
		}
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