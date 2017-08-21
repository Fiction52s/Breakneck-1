#ifndef __AURA_H__
#define __AURA_H__

#include "SFML\Graphics.hpp"
#include <list>

struct Actor;
struct Tileset;

struct Aura
{
	struct AuraParams
	{
		enum ParamType
		{
			BASIC,
			NORMAL
		};

		AuraParams( ParamType pt )
			:pType( pt )
		{

		}
		
		ParamType pType;
	};

	struct NormalParams : AuraParams
	{
		NormalParams()
			:AuraParams( AuraParams::NORMAL )
		{

		}
		sf::Vector2f centerPos;
	};

	struct ParticleSet;
	struct Particle
	{
		Particle( ParticleSet *ps, sf::Vertex *va, int ttl, sf::IntRect &sub );
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
		ParticleSet *ps;
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
		AuraParams *ap;
		//void CheckActive();
		bool actuallyDone;
	};



	int totalParticles;
	Aura( Actor *player, int numSets, int maxParticlesPerSet );
	void Update();
	void Draw(sf::RenderTarget *target);
	//void SetParticleSet(std::list<sf::Vector2f> &points);

	void ActivateParticles(std::list<sf::Vector2f> &points, 
		sf::Transform &tr, const sf::Vector2f &origin, AuraParams *ap );
	static void CreateParticlePointList(Tileset *ts, sf::Image &im, 
		int tileIndex,
		std::list<sf::Vector2f> &outPoints );

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