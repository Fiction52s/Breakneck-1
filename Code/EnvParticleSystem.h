#ifndef __ENV_PARTICLE_SYSTEM
#define __ENV_PARTICLE_SYSTEM

#include "SFML\Graphics.hpp"
#include <vector>

struct EnvParticleSystem
{
	//std::vector<


	EnvParticleSystem();
	~EnvParticleSystem();
	void Update();
	void Draw( int effectLayer, sf::RenderTarget *target);
};

#endif