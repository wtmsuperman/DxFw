#ifndef __NODE__
#define __NODE__
#include "mathlib.h"
#include <map>
#include <dx/dx_defines.h>
#include <dx/dx_logging.h>

class Node
{
public:
	enum TransformSpace
	{
		TS_PARENT,
		TS_LOCAL,
		TS_WORLD
	};
protected:
	Vector3 pos;
	Quaternion orientation;
	Node*	parent;
	
	typedef std::map<std::string,Node*>		NodeMap;
	typedef NodeMap::iterator						NodeIter;
	typedef NodeMap::const_iterator					ConstNodeIter;

	NodeMap											mNodes;
	char*											name;
public:

	Node()
		:pos(0.0f,0.0f,0.0f),orientation(1.0f,0.0f,0.0f,0.0f),parent(0)
	{
		name = new char[5];
		strcpy(name,"null");
	}

	Node(const char* name)
		:pos(0.0f,0.0f,0.0f),orientation(1.0f,0.0f,0.0f,0.0f),parent(0)
	{
		this->name = new char[strlen(name)+1];
		strcpy(this->name,name);
	}

	~Node()
	{
		if (parent)
		{
			parent->removeChild(name);
		}
		safe_deleteArray(name);
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

	void lookAt(const Vector3& position)
	{
		orientation.setToRatationTo(Vector3::UNIT_Z,position - this->pos);
	}

	void lookAt(float x,float y,float z)
	{
		orientation.setToRatationTo(Vector3::UNIT_Z,Vector3(x,y,z) - this->pos);
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
		case TS_WORLD:
			orientation = orientation * conjugate(getDerivedOrientation()) * qn * getDerivedOrientation();
			break;
		};
		//orientation = orientation * qn;
	}

	

	void translate(float x,float y,float z,TransformSpace ts = TS_WORLD)
	{
		translate(Vector3(x,y,z),ts);
	}

	void translate(const Vector3& v,TransformSpace ts = TS_WORLD)
	{
		switch (ts)
		{
		case TS_PARENT:
			pos += v;
			break;
		case TS_LOCAL:
			pos += v * Matrix3x3(orientation);
			break;
		case TS_WORLD:
			if (parent)
			{
				Vector3 v2= v * Matrix3x3(conjugate(parent->getDerivedOrientation()));
				pos += v2;
			}
			else
			{
				pos += v;
			}
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


	//Ogre中使用了catche保存了值，只有发送变化时才重新计算
	//以后进行更改，加入缓存，应该能显著降低计算量
	Quaternion getDerivedOrientation() const
	{
		if (parent)
		{
			//将来进行更改，可以继承父类的旋转
			return orientation;
		}
		else
		{
			return orientation;
		}
	}

	Vector3 getDerivedPosition() const
	{
		if (parent)
		{
			Matrix3x3 r(getDerivedOrientation());
			return parent->getDerivedPosition() + (pos * r);
		}
		else
		{
			return pos;
		}
	}

	void generateLocalToParentMatrix(Matrix4x4* out)
	{
		out->setupLocalToParent(pos,orientation);
	}

	void generateLocalToWorldMatrix(Matrix4x4* out)
	{
		out->setupLocalToParent(getDerivedPosition(),getDerivedOrientation());
	}

	void generateParentToLocalMatrix(Matrix4x4* out)
	{
		out->setupParentToLocal(pos,orientation);
	}

	void generateWorldToLocalMatrix(Matrix4x4* out)
	{
		out->setupParentToLocal(getDerivedPosition(),getDerivedOrientation());
	}

	virtual  Node* createChildImpl(const char* name)
	{
		return new Node(name);
	}

	virtual Node* createChildImpl()
	{
		return new Node;
	}

	Node*		createChild(const char* name);
	Node*		createChild();
	Node*		removeChild(const char* name);
	Node*		getChild(const char* name);
	void		destroyChild(const char* name);

	void		destroyAllChild();
};

#endif