#ifndef __SCRPIT_PROPERTY__
#define __SCRPIT_PROPERTY__

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <assert.h>

using std::map;
using std::string;

/*******
//  ����һЩ�򵥵����Խű�
//
//  �������������ǻ������ͣ���Ӧ����ָ������
//  
*///////

class PropertySrcipt
{

public:
	PropertySrcipt()
	{};
	~PropertySrcipt();

	bool loadScript(const string& fileName);

	void clear();

	//�����ַ������������͵����ݣ���Ӧ��ʹ�ø÷�����ʹ��getAsString,getAsArray���
	template<class T>
	T get(const string& name)
	{
		assert(mData.count(name) != 0);
		return *static_cast<T*>(mData.find(name)->second);
	}

	char* getAsString(const string& name)
	{
		assert(mData.count(name) != 0);

		return static_cast<char*>(mData.find(name)->second);
	}

	void getAsString(const string& name,char* output)
	{
		assert(mData.count(name) != 0);
		strcpy(output,static_cast<const char*>(mData.find(name)->second));
	}

	//��ȡvoidָ�����ͣ�����������ת��
	void* getAsPtr(const string& name)
	{
		assert(mData.count(name) != 0);
		return mData.find(name)->second;
	}


	template<class T>
	void getAsArray(const string& name,size_t len,T* out)
	{
		assert(mData.count(name) != 0);
		T* p = static_cast<T*>(mData.find(name)->second);

		for (size_t i = 0; i < len; i++)
		{
			*out++ = *static_cast<T*>(p++);
		}
	}

	template<class T>
	void addArray(const string& name,T* p,size_t size)
	{
		size_t block = sizeof(T) * size;
		T* t = static_cast<T*>(malloc(block));
		memcpy(t,p,block);

		property_iter iter = mData.find(name);
		if (iter != mData.end())
		{
			free(iter->second);
			iter->second = t;
		}
		else
		{
			mData[name] = t;
		}
	}

	template<class T>
	void setArray(const string& name,T* p,size_t size)
	{
		property_iter iter = mData.find(name);
		if (iter != mData.end())
		{
			free(iter->second);

			size_t block = sizeof(T) * size;
			T* t = static_cast<T*>(malloc(block));
			memcpy(t,p,block);

			*t = val;
			iter->second = t;
		}
	}

	template<class T>
	void addVariable(const string& name,const T& val)
	{
		T* t = static_cast<T*>(malloc(sizeof(T)));
		*t = val;

		property_iter iter = mData.find(name);
		if (iter != mData.end())
		{
			free(iter->second);
			iter->second = t;
		}
		else
		{
			mData[name] = t;
		}
	}

	//char*��ƫ�ػ��汾
	void addVariable(const string& name,const char* val)
	{
		size_t l = strlen(val);
		char* t = static_cast<char*>(malloc(l+1));
		strcpy(t,val);

		property_iter iter = mData.find(name);
		if (iter != mData.end())
		{
			free(iter->second);
			iter->second = t;
		}
		else
		{
			mData[name] = t;
		}
	}

	template<class T>
	void setVariable(const string& name,const T& val)
	{
		property_iter iter = mData.find(name);
		if (iter != mData.end())
		{
			free(iter->second);
			T* t = static_cast<T*>(malloc(sizeof(T)));
			*t = val;
			iter->second = t;
		}
	}

	void setVariable(const string& name,const char* val)
	{
		property_iter iter = mData.find(name);
		if (iter != mData.end())
		{
			free(iter->second);
			size_t l = strlen(val);
			char* t = static_cast<char*>(malloc(l+1));
			strcpy(t,val);
			iter->second = t;
		}
	}

private:
	map<std::string,void*> mData;
	typedef map<std::string,void*>::iterator property_iter;
	typedef map<std::string,void*>::value_type property_type;
};

#endif