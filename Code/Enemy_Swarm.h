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
		Count
	};

	Action action;
	SwarmMember(Swarm *parent,
		sf::VertexArray &va, int index,
		sf::Vector2<double> &targetOffset,
		double p_maxSpeed);
	void ClearSprite();
	void UpdateSprite();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ResetEnemy();
	void HandleNoHealth();
	void FrameIncrement();

	int actionLength[Count];
	int animFactor[Count];

	int framesToLive;

	Swarm *parent;
	double maxSpeed;
	int vaIndex;
	sf::Vector2<double> targetOffset;

	sf::Vector2<double> velocity;

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
		Count
	};

	Swarm(GameSession *owner,
		sf::Vector2i &pos, bool p_hasMonitor,
		int p_level );
	~Swarm();
	void ActionEnded();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void HandleNoHealth();

	void Launch();
	void ResetEnemy();

	int animationFactor;
	int actionLength[Action::Count];
	int animFactor[Action::Count];
	Action action;
	int liveFrames;

	sf::Vector2<double> origPosition;
	bool dying;
	Tileset *ts;
	Tileset *ts_swarm;

	const static int NUM_SWARM = 5;
	sf::VertexArray swarmVA;
	sf::Sprite nestSprite;
	SwarmMember *members[NUM_SWARM];

	Tileset *ts_swarmExplode;

	sf::Vector2f spriteSize;
	double maxSpeed;
};

#endif