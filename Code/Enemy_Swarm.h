#ifndef __ENEMY_SWARM_H__
#define __ENEMY_SWARM_H__

#include "Enemy.h"

struct Swarm;
struct SwarmMember : Enemy
{
	enum Action
	{
		FLY,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int framesToLive;
		V2d velocity;
	};
	MyData data;


	V2d targetOffset;
	Swarm *parent;
	double maxSpeed;
	int vaIndex;
	int swarmTypeIndex;

	double bulletSpeed;

	SwarmMember(Swarm *parent,
		int index,
		V2d &targetOffset,
		double p_maxSpeed);
	void ClearSprite();
	void UpdateSprite();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ResetEnemy();
	void HandleNoHealth();
	void FrameIncrement();
	void Throw(V2d &pos);
	int GetNumEnergyAbsorbParticles();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	
};

struct Swarm : Enemy
{
	enum Action
	{
		NEUTRAL,
		FIRE,
		USED,
		REFILL,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		bool dying;
	};
	MyData data;

	int liveFrames;

	
	Tileset *ts;
	Tileset *ts_swarm;

	const static int NUM_SWARM = 5;
	sf::Vertex swarmVA[NUM_SWARM * 4];
	SwarmMember *members[NUM_SWARM];

	Tileset *ts_swarmExplode;

	sf::Vector2f spriteSize;
	double maxSpeed;



	Swarm(ActorParams *ap);
	~Swarm();
	void SetLevel(int lev);
	void ActionEnded();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void HandleNoHealth();
	int GetNumEnergyAbsorbParticles();
	int GetTotalEnergyAbsorbParticles();
	void Launch();
	void ResetEnemy();
	void AddToGame();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif