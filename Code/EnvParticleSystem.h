#ifndef __ENV_PARTICLE_SYSTEM
#define __ENV_PARTICLE_SYSTEM

#include "SFML\Graphics.hpp"
#include <vector>

struct Session;
struct ShapeEmitter;

struct EnvParticleSystem
{
	std::vector<ShapeEmitter*> emitters;
	Session *sess;

	EnvParticleSystem();
	~EnvParticleSystem();
	void Update();
	//void LayeredDraw( int p_drawLayer, sf::RenderTarget *target);
};

#endif