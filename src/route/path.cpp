#include "route/path.h"

//
// LinearPath
//

LinearPath::LinearPath(const Vector3& start,const Vector3& end,float d)
	:startPosition(start),endPosition(end),duration(d),currentTime(0.0f)
{}

LinearPath::~LinearPath()
{}


//////////////////////////////////////////////////////////////////////////////