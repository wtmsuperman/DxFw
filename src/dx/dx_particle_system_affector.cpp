#include <dx/dx_particle_system_affector.h>
#include <dx/dx_particle_system.h>

LinearForceAffector::LinearForceAffector(const Vector3& Force,ForceType Type)
	:force(Force),type(Type)
{

}

LinearForceAffector::LinearForceAffector(float x,float y,float z,ForceType Type)
	:force(x,y,z),type(Type)
{}

void LinearForceAffector::init(DxParticleAttribute* particle)
{
	//do nothing;
}

void LinearForceAffector::affect(DxParticleAttribute* particle,float timeDelta)
{
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

ColorFaderAffector::ColorFaderAffector(float a,float r,float g,float b)
{
	this->a = clamp(a,-1.0f,1.0f);
	this->r = clamp(r,-1.0f,1.0f);
	this->g = clamp(g,-1.0f,1.0f);
	this->b = clamp(b,-1.0f,1.0f);
}

void ColorFaderAffector::init(DxParticleAttribute* p)
{
	//do nothing
}

void ColorFaderAffector::affect(DxParticleAttribute* p,float timeDelta)
{
	p->color.a = clamp(p->color.a + a*timeDelta,0.0f,1.0f);
	p->color.r = clamp(p->color.r + r*timeDelta,0.0f,1.0f);
	p->color.g = clamp(p->color.g + g*timeDelta,0.0f,1.0f);
	p->color.b = clamp(p->color.b + b*timeDelta,0.0f,1.0f);

}

void ColorFaderAffector::setColor(float a,float r,float g,float b)
{
	this->a = clamp(a,-1.0f,1.0f);
	this->r = clamp(r,-1.0f,1.0f);
	this->g = clamp(g,-1.0f,1.0f);
	this->b = clamp(b,-1.0f,1.0f);
}

void ColorFaderAffector::setColor(const DxColorValue& colorFade)
{
	this->a = clamp(colorFade.a,-1.0f,1.0f);
	this->r = clamp(colorFade.r,-1.0f,1.0f);
	this->g = clamp(colorFade.g,-1.0f,1.0f);
	this->b = clamp(colorFade.b,-1.0f,1.0f);
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