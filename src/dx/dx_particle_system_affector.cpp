#include <dx/dx_particle_system_affector.h>
#include <dx/dx_particle_system.h>
#include <script/scriptlib.h>
#include <map>

LinearForceAffector::LinearForceAffector()
{
	force = Vector3::ZERO;
	type = FT_ADD;
}

LinearForceAffector::LinearForceAffector(const Vector3& Force,ForceType Type)
	:force(Force),type(Type)
{

}

LinearForceAffector::LinearForceAffector(float x,float y,float z,ForceType Type)
	:force(x,y,z),type(Type)
{}

void LinearForceAffector::init(DxParticleSystem* ps)
{
	//do nothing;
}

void LinearForceAffector::affect(DxParticleSystem* ps,float timeDelta)
{
	DxParticleSystem::ActiveParticleListIter end = ps->activeParticleEnd();

	if (FT_ADD)
	{
		Vector3 r = force * timeDelta;
	}

	for (DxParticleSystem::ActiveParticleListIter iter = ps->activeParticleBegin(); iter != end; ++iter)
	{
		DxParticleAttribute* particle = *iter;
		switch (type)
		{
		case FT_ADD:
			particle->velocity += force * timeDelta;
			break;
		case FT_AVERAGE:
			particle->velocity = (particle->velocity + force) * 0.5f;
			break;
		}
	}
}

DxParticleAffector* createLinearForceAffector(const char* file)
{
	lua_State* L = lua_open();
	if (luaL_loadfile(L,file))
	{
		assert(true && "affector script path error");
		lua_close(L);
		return 0;
	}
	lua_pcall(L,0,0,0);
	lua_getglobal(L,"affector");
	if (!lua_istable(L,-1))
	{
		lua_close(L);
		return 0;
	}

	lua_getfield(L,-1,"force");
	if (!lua_istable(L,-1))
	{
		lua_close(L);
		return 0;
	}
	lua_rawgeti(L,-1,1);
	lua_rawgeti(L,-2,2);
	lua_rawgeti(L,-3,3);
	Vector3 force((float)lua_tonumber(L,-3),(float)lua_tonumber(L,-2),(float)lua_tonumber(L,-1));
	lua_pop(L,1);
	lua_pop(L,1);
	lua_pop(L,1);
	lua_pop(L,1);

	lua_getfield(L,-1,"type");
	const char* typeStr = lua_tostring(L,-1);
	lua_pop(L,1);

	LinearForceAffector::ForceType type;

	if (strcmp("add",typeStr)==0)
	{
		type = LinearForceAffector::FT_ADD;
	}
	else if (strcmp("average",typeStr) == 0)
	{
		type = LinearForceAffector::FT_AVERAGE;
	}

	LinearForceAffector* affector = new LinearForceAffector(force,type);
	lua_close(L);
	return affector;
}

ColorFaderAffector::ColorFaderAffector()
{
	a = 0.0f;
	r = 0.0f;
	g = 0.0f;
	b = 0.0f;
}

ColorFaderAffector::ColorFaderAffector(float a,float r,float g,float b)
{
	this->a = a;
	this->b = b;
	this->g = g;
	this->r = r;
}

void ColorFaderAffector::init(DxParticleSystem* ps)
{
	//do nothing
}

void ColorFaderAffector::affect(DxParticleSystem* ps,float timeDelta)
{
	DxParticleSystem::ActiveParticleListIter end = ps->activeParticleEnd();

	for (DxParticleSystem::ActiveParticleListIter iter = ps->activeParticleBegin(); iter != end; ++iter)
	{
		DxParticleAttribute* p = *iter;
		p->color.a = clamp(p->color.a + a*timeDelta,0.0f,1.0f);
		p->color.r = clamp(p->color.r + r*timeDelta,0.0f,1.0f);
		p->color.g = clamp(p->color.g + g*timeDelta,0.0f,1.0f);
		p->color.b = clamp(p->color.b + b*timeDelta,0.0f,1.0f);
	}
}

void ColorFaderAffector::setColor(float a,float r,float g,float b)
{
	this->a = a;
	this->b = b;
	this->g = g;
	this->r = r;
}

void ColorFaderAffector::setColor(const DxColorValue& colorFade)
{
	this->a = colorFade.a;
	this->r = colorFade.r;
	this->g = colorFade.g;
	this->b = colorFade.b;
}

void ColorFaderAffector::getColor(float* a,float* r,float* g,float* b)
{
	*a = this->a;
	*r = this->r;
	*g = this->g;
	*b = this->b;
}

void ColorFaderAffector::getColor(DxColorValue* color)
{
	color->a = a;
	color->r = r;
	color->g = g;
	color->b = b;
}

DxColorValue ColorFaderAffector::getColor()
{
	DxColorValue v;
	v.a = a;
	v.r = r;
	v.g = g;
	v.b = b;
	return v;
}

static std::map<std::string,AffectorCreator> creators;

void registAffectorCreator(const char* name,AffectorCreator creator)
{
	creators[name] = creator;
}

AffectorCreator getAffectorCreator(const char* name)
{
	std::map<std::string,AffectorCreator>::iterator iter;
	iter = creators.find(name);
	return iter != creators.end() ? iter->second : 0;
}

void registAllDefaultAffectos()
{
	registAffectorCreator("LinearForce",createLinearForceAffector);
	registAffectorCreator("ColorFader",createColorFaderAffector);
}

DxParticleAffector* createColorFaderAffector(const char* file)
{
	lua_State* L = lua_open();
	if (luaL_loadfile(L,file))
	{
		assert(true && "affector script path error");
		lua_close(L);
		return 0;
	}
	lua_pcall(L,0,0,0);
	
	lua_getglobal(L,"affector");
	if (lua_isnil(L,-1))
	{
		lua_close(L);
		return 0;
	}

	ColorFaderAffector* colorFader = new ColorFaderAffector();
	lua_getfield(L,-1,"a");
	lua_getfield(L,-2,"r");
	lua_getfield(L,-3,"g");
	lua_getfield(L,-4,"b");
	if (lua_isnumber(L,-1) && lua_isnumber(L,-2) && lua_isnumber(L,-3) && lua_isnumber(L,-4))
	{
		colorFader->setColor((float)lua_tonumber(L,-4),(float)lua_tonumber(L,-3),(float)lua_tonumber(L,-2),(float)lua_tonumber(L,-1));
	}
	else
	{
		lua_close(L);
		return 0;
	}
	lua_close(L);
	return colorFader;
}