#ifndef __ABSORBPARTICLES_H__
#define __ABSORBPARTICLES_H__

#include <SFML\Graphics.hpp>
#include "VectorMath.h"

struct Session;
struct Actor;
struct Tileset;

struct AbsorbParticles
{
	enum AbsorbType : int
	{
		ENERGY,
		DARK,
		SHARD,
	};
	
	struct SingleEnergyParticle
	{
		//36 bytes. kinda sucks. could make it 32 with some packing
		struct MyData
		{
			sf::Vector2f pos;
			sf::Vector2f velocity;
			int frame;
			int lockFrame;
			float lockDist;
			SingleEnergyParticle *next;
			SingleEnergyParticle *prev;
			Actor *playerTarget;
		};

		MyData data;
		int tileIndex;
		AbsorbParticles *parent;

		SingleEnergyParticle(AbsorbParticles *parent,
			int tileIndex );
		void UpdateSprite();
		bool Update();
		void Activate( Actor *p_playerTarget, sf::Vector2f &pos, sf::Vector2f &vel);
		void Clear();
		int GetNumStoredBytes();
		void StoreBytes(unsigned char *bytes);
		void SetFromBytes(unsigned char *bytes);
		sf::Vector2f GetTargetPos(AbsorbType ab);

	};

	struct MyData
	{
		SingleEnergyParticle *activeList;
		SingleEnergyParticle *inactiveList;
		bool directKilled;
	};
	MyData data;


	std::vector<SingleEnergyParticle*> allParticles;
	sf::Vertex *va;
	int maxNumParticles;
	Session *sess;
	Tileset *ts;
	Tileset *ts_explodeDestroy;
	int animFactor;
	double maxSpeed;
	AbsorbType abType;

	void KillAllActive();
	AbsorbParticles(Session *sess,
		AbsorbType p_abType );
	~AbsorbParticles();	
	void Reset();
	
	void Activate( Actor *playerTarget, int storedHits, 
		V2d &pos,
		float startAngle = 0 );
	void Update();
	void Draw(sf::RenderTarget *rt);
	SingleEnergyParticle *GetInactiveParticle();
	void DeactivateParticle(SingleEnergyParticle *sp);
	void AllocateParticle(int tileIndex );
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif