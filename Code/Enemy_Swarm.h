#ifndef __ENEMY_SWARM_H__
#define __ENEMY_SWARM_H__

#include "Enemy.h"

struct Swarm;
struct SwarmMember : Enemy
{
	enum Action
	{
		FLY,
		DIE,
		A_Count
	};

	SwarmMember(Swarm *parent,
		sf::VertexArray &va, int index,
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

	int framesToLive;

	Swarm *parent;
	double maxSpeed;
	int vaIndex;
	V2d targetOffset;

	V2d velocity;

	bool active;

	sf::VertexArray &va;

	double bulletSpeed;
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

	Swarm(ActorParams *ap);
	~Swarm();
	void SetLevel(int lev);
	void ActionEnded();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void HandleNoHealth();

	void Launch();
	void ResetEnemy();

	int liveFrames;

	bool dying;
	Tileset *ts;
	Tileset *ts_swarm;

	const static int NUM_SWARM = 5;
	sf::VertexArray swarmVA;
	SwarmMember *members[NUM_SWARM];

	Tileset *ts_swarmExplode;

	sf::Vector2f spriteSize;
	double maxSpeed;
};

#endif