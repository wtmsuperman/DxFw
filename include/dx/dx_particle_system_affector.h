#ifndef __DX_PARTICLE_SYSTEM_AFFECTOR__
#define __DX_PARTICLE_SYSTEM_AFFECTOR__

#include <vector3.h>
#include <dx/dx_defines.h>
#include <dx/dx_particle_system_affector.h>

class DxParticleSystem;

// Regist all the provided affectors
// Call this function befor load particle system
void registAllDefaultAffectos();


//Abstract class define a Particle Affector
//Particle affector couble be the linear force such as gravity,or the color fader.
//You can define your own Affector by extending this class
class DxParticleAffector
{
public:
	virtual void init(DxParticleSystem* ps) = 0;
	virtual void affect(DxParticleSystem* ps,float timeDelta) = 0;
};

// Affector creator

typedef DxParticleAffector* (*AffectorCreator)(const char* file);

// Regist an affector creator by given name
// You should regist a manual affector,then you can use it in your particle system script file.
// Like:
//
// In the affector.lua
// affecotr={
//	 name="myaffector",
//   ...
//   ...
//   ...
// }
//
// In your .cpp
// you should call registAffectorCreator("myaffector",func) befor you load the scrpit
// func is your affecotr creator,take the lua script path

void registAffectorCreator(const char* name,AffectorCreator creator);


// Get a affector creator

AffectorCreator getAffectorCreator(const char* name);

/** This class defines a ParticleAffector which applies a linear force to particles in a system.
    @remarks
        This affector (see ParticleAffector) applies a linear force, such as gravity, to a particle system.
        This force can be applied in 2 ways: by taking the average of the particle's current momentum and the 
        force vector, or by adding the force vector to the current particle's momentum. 
    @par
        The former approach is self-stabilising i.e. once a particle's momentum
        is equal to the force vector, no further change is made to it's momentum. It also results in
        a non-linear acceleration of particles.
        The latter approach is simpler and applies a constant acceleration to particles. However,
        it is not self-stabilising and can lead to perpetually increasing particle velocities. 
*/
class LinearForceAffector : public DxParticleAffector
{
public:

	enum ForceType
	{
		FT_ADD,
		FT_AVERAGE
	};

	LinearForceAffector();
	LinearForceAffector(const Vector3& force,ForceType type=FT_ADD);
	LinearForceAffector(float x,float y,float z,ForceType type=FT_ADD);

	virtual void init(DxParticleSystem* ps);
	virtual void affect(DxParticleSystem* ps,float timeDelta);

	Vector3		force;
	ForceType	type;
};

// change color per time delta

class ColorFaderAffector : public DxParticleAffector
{
public:

	ColorFaderAffector();

	ColorFaderAffector(float a,float r,float g,float b);

	virtual void init(DxParticleSystem* ps);
	virtual void affect(DxParticleSystem* ps,float timeDelta);

	void setColor(float a,float r,float g,float b);
	void setColor(const DxColorValue& colorFade);

	void			getColor(float* a,float* r,float* g,float* b);
	void			getColor(DxColorValue* colorFade);
	DxColorValue	getColor();


private:
	float a;
	float r;
	float g;
	float b;
};


//* Affector creator

extern DxParticleAffector* createColorFaderAffector(const char* file);
extern DxParticleAffector* createLinearForceAffector(const char* file);

///////////////////////////////////////////////////////////////////////////

#endif