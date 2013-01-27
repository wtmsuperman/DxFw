#ifndef __NODE__
#define __NODE__
#include "mathlib.h"


class Node
{
public:
	enum TransformSpace
	{
		TS_PARENT,
		TS_LOCAL
	};
protected:
	Vector3 pos;
	Quaternion orientation;
public:

	Node()
		:pos(0.0f,0.0f,0.0f),orientation(1.0f,0.0f,0.0f,0.0f)
	{

	}

	void setPosition(const Vector3& pos)
	{
		this->pos = pos;
	}

	void setPosition(float x,float y,float z)
	{
		pos.x = x;
		pos.y = y;
		pos.z = z;
	}

	void setOrientation(const Quaternion& q)
	{
		orientation = q;
	}

	void setOrientation(const EulerAngles& e)
	{
		orientation.setToRotateInertialToObject(e);
	}

	void lookAt(const Vector3& pos)
	{
		orientation.setToRatationTo(Vector3::UNIT_Z,pos-this->pos);
	}

	void yaw(float theta)
	{
		rotate(Vector3::UNIT_Y,theta);
	}

	void pitch(float theta)
	{
		rotate(Vector3::UNIT_X,theta);
	}

	void roll(float theta)
	{
		rotate(Vector3::UNIT_Z,theta);
	}

	void rotate(const Vector3& axis,float angle,TransformSpace ts = TS_LOCAL)
	{
		Quaternion q;
		q.setToRotateAxis(axis,angle);
		rotate(q,ts);
	}

	void rotate(const Quaternion& q,TransformSpace ts = TS_LOCAL)
	{
		Quaternion qn = q;
		qn.normalize();
		switch (ts)
		{
		case TS_PARENT:
			orientation = qn * orientation;
			break;
		case TS_LOCAL:
			orientation = orientation * qn;
			break;
		};
		//orientation = orientation * qn;
	}

	void generateLocalToParentMatrix(Matrix4x4* out)
	{
		out->setupLocalToParent(pos,orientation);
	}

	void generateParentToLocalMatrix(Matrix4x4* out)
	{
		out->setupParentToLocal(pos,orientation);
	}

	void translate(float x,float y,float z,TransformSpace ts = TS_PARENT)
	{
		translate(Vector3(x,y,z),ts);
	}

	void translate(const Vector3& v,TransformSpace ts = TS_PARENT)
	{
		switch (ts)
		{
		case TS_PARENT:
			pos += v;
			break;
		case TS_LOCAL:
			pos += v * Matrix3x3(orientation);
			break;
		}
	}

	const Vector3& getPosition() const
	{
		return pos;
	}

	const Quaternion& getOrientation() const
	{
		return orientation;
	}


};

#endif