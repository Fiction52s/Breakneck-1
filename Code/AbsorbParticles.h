#ifndef __ABSORBPARTICLES_H__
#define __ABSORBPARTICLES_H__

#include <SFML\Graphics.hpp>


struct GameSession;
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
		
		SingleEnergyParticle(AbsorbParticles *parent,
			int tileIndex );
		void UpdateSprite();
		bool Update();
		void Activate(sf::Vector2f &pos, sf::Vector2f &vel);
		void Clear();
		sf::Vector2f pos;
		int frame;
		int tileIndex;
		sf::Vector2f velocity;
		AbsorbParticles *parent;
		int lockFrame;
		float lockDist;

		SingleEnergyParticle *next;
		SingleEnergyParticle *prev;
		
		//Vector2f accel;
	};

	void KillAllActive();
	bool directKilled;
	AbsorbType abType;
	sf::Vector2f GetTargetPos(AbsorbType ab);
	AbsorbParticles( GameSession *owner,
		AbsorbType p_abType );
	~AbsorbParticles();	
	void Reset();
	sf::Vertex *va;
	int maxNumParticles;
	void Activate( Actor *playerTarget, int storedHits, 
		sf::Vector2<double> &pos,
		float startAngle = 0 );
	void Update();
	void Draw(sf::RenderTarget *rt);
	float startAngle;
	GameSession *owner;
	Tileset *ts;
	//Tileset *ts_explodeCreate;
	Tileset *ts_explodeDestroy;
	int animFactor;
	sf::Vector2f pos;
	sf::Vector2f *particlePos;
	int numActivatedParticles;
	Actor *playerTarget;
	double maxSpeed;
	SingleEnergyParticle *GetInactiveParticle();
	void DeactivateParticle(SingleEnergyParticle *sp);
	SingleEnergyParticle *activeList;
	SingleEnergyParticle *inactiveList;
	void AllocateParticle(int tileIndex );
};

#endif