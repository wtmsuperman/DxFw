#include "script_property.h"
#include <fstream>

#define SCRIPT_VAL_TYPE unsigned char

const SCRIPT_VAL_TYPE TYPE_INT = 0x01;
const SCRIPT_VAL_TYPE TYPE_FLOAT = 0x01 << 1;
const SCRIPT_VAL_TYPE TYPE_STR = 0x01 << 2;

const SCRIPT_VAL_TYPE TYPE_IS_ARRAY = 0x80;
const SCRIPT_VAL_TYPE TYPE_NONE_ARRAY = 0x4f;

SCRIPT_VAL_TYPE determType(std::string::iterator& beg,std::string::iterator& end)
{
	unsigned char type = 0;
	std::string::iterator iter = beg;
	while (iter != end && *iter != ' ')
		++iter;

	if (iter != end)
		type |= TYPE_IS_ARRAY;

	iter = beg;

	if (isdigit(*iter) == 0)
	{
		return type | TYPE_STR;
	}

	while (iter != end && *iter != '.')
		++iter;

	if (iter != end)
		type |= TYPE_FLOAT;
	else
		type |= TYPE_INT;

	return type;
}

bool PropertySrcipt::loadScript(const string& fileName)
{
	std::ifstream ifs;
	ifs.open(fileName);

	if (!ifs.is_open())
	{
		return false;
	}

	while (! ifs.eof())
	{
		std::string tempstr;
		std::getline(ifs,tempstr);

		if (tempstr.empty())
			continue;

		std::string::iterator iter = tempstr.begin();
		std::string::iterator endIter = tempstr.end();

		//过滤开头空格
		while (iter != endIter && *iter == ' ')
			++iter;
		//跳过注释
		if (iter != endIter && *iter == '#')
			continue;

		std::string::iterator keyIter = iter;
		while (iter != endIter && *iter != ' ')
		{
			++iter;
		}
		std::string key(keyIter,iter);

		//过滤空格
		while (iter != endIter && *iter ==' ')
			++iter;

		std::string::iterator valueIterBeg = iter;
		
		std::string::iterator valueIter = valueIterBeg;

		SCRIPT_VAL_TYPE type = determType(valueIter,endIter);
		std::string v(valueIter,endIter);
		if (! (type & TYPE_IS_ARRAY)) //非数组
		{
			if (type == TYPE_STR)
			{
				addVariable(key,v.c_str());
			}
			else if (type == TYPE_INT)
			{
				addVariable(key,atoi(v.c_str()));
			}
			else if (type == TYPE_FLOAT)
			{
				addVariable(key,atof(v.c_str()));
			}
		}
		else
		{
			type &= TYPE_NONE_ARRAY;
			std::vector<string> vals;
			std::string::iterator tmpIter = valueIter;
			while (valueIter != endIter)
			{
				if (*valueIter == ' ')
				{
					vals.push_back(std::string(tmpIter,valueIter));
					tmpIter = valueIter+1;
				}
				++valueIter;
			}
			if (tmpIter != endIter)
			{
				vals.push_back(std::string(tmpIter,endIter));
			}
			if (type ==  TYPE_FLOAT)
			{
				std::vector<double> vf;
				size_t l = vals.size();
				for (size_t i=0; i < l; ++i)
				{
					vf.push_back(atof(vals[i].c_str()));
				}
				addArray(key,&vf[0],l);
				///return true;
			}
			else if (type == TYPE_INT)
			{
				std::vector<int> vi;
				size_t l = vals.size();
				for (size_t i=0; i < l; ++i)
				{
					vi.push_back(atoi(vals[i].c_str()));
				}
				addArray(key,&vi[0],l);
				//return true;
			}
		}
	}

	return true;
}

void PropertySrcipt::clear()
{
	property_iter iter = mData.begin();
	property_iter end = mData.end();

	while (iter != end)
	{
		free(iter->second);
		iter++;
	}

	mData.clear();
}

PropertySrcipt::~PropertySrcipt()
{
	clear();
}