#ifndef __PARTICLE_HANDLER_H__
#define __PARTICLE_HANDLER_H__

struct ShapeParticle;

struct ParticleHandler
{
	virtual void UpdateShapeParticle(ShapeParticle *sp) = 0;
	virtual void ActivateShapeParticle(ShapeParticle *sp) = 0;
};

#endif 