#ifndef __AURA_H__
#define __AURA_H__

#include "SFML\Graphics.hpp"
#include <list>

struct Actor;
struct Tileset;

struct Aura
{
	struct Particle
	{
		Particle(sf::Vertex *va, int ttl, sf::IntRect &sub );
		void Set(sf::Vector2f &pos, sf::Vector2f &vel,
			sf::Vector2f &accel);
		void Update();
		void Clear();
		sf::Vector2f pos;
		sf::Vector2f vel;
		sf::Vector2f accel;
		int ttl;
		int maxTTL;
		sf::Vertex *quad;
		bool active;
	};

	struct ParticleSet
	{
		ParticleSet(Aura *aura, int index);
		int index;
		ParticleSet *next;
		ParticleSet *prev;
		void Activate();
		void Update();
		int frame;
		int maxFramesToLive;
		Aura *aura;
		Particle **particles;
		int numParticlesFromSprite;
	};



	int totalParticles;
	Aura( Actor *player, int numSets, int maxParticlesPerSet );
	void Update();
	void Draw(sf::RenderTarget *target);
	//void SetParticleSet(std::list<sf::Vector2f> &points);

	void ActivateParticles(std::list<sf::Vector2f> &points, sf::Transform &tr );
	static void CreateParticlePointList(Tileset *ts, sf::Image &im, 
		int tileIndex,
		std::list<sf::Vector2f> &outPoints, sf::Vector2f &origin );

	sf::Vertex *va;
	int numParticleSets;
	int maxParticlesPerSet;
	Actor *player;

	void DeactivateParticles(ParticleSet *ps);
	void AddToInactive(ParticleSet *ps);

	Tileset *ts;

	ParticleSet *inactiveSets;
	ParticleSet *activeSets;

};

#endif