#include <dx/dx_particle_system_affector.h>
#include <dx/dx_particle_system.h>
#include <script/scriptlib.h>
#include <map>

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

void registAffectorCreator(const char* name,AffecotrCreator creator)
{
	static std::map<std::string,AffecotrCreator> creators;
	creators[name] = creator;
}