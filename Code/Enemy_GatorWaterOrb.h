#ifndef __ENEMY_GATORWATERORB_H__
#define __ENEMY_GATORWATERORB_H__

#include "Enemy.h"
#include <vector>

struct GatorWaterOrb;

struct GatorWaterOrbPool
{
	GatorWaterOrbPool();
	~GatorWaterOrbPool();
	void Reset();
	GatorWaterOrb * Throw(V2d &pos, V2d &dir,
		int orbType );
	void Draw(sf::RenderTarget *target);
	std::vector<GatorWaterOrb*> bulletVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numBullets;
};

struct GatorWaterOrb : Enemy
{
	enum OrbType
	{
		UNDODGEABLE_REFRESH,
		NODE_GROW,
	};

	enum Action
	{
		FLYING,
		A_Count
	};

	int orbType;

	Tileset *ts;

	double flySpeed;
	double maxFlySpeed;
	double accel;
	int framesToLive;
	int origFramesToLive;
	V2d velocity;
	double distToTarget;

	sf::Vertex *quad;

	//for node grow
	double currRadius;
	V2d targetPos;

	QuadraticMovement *quadraticMove;
	MovementSequence quadraticMoveSeq;

	

	GatorWaterOrb(sf::Vertex *quad,
		GatorWaterOrbPool *pool);
	void UpdateEnemyPhysics();
	void Throw(V2d &pos, V2d &dir, int orbType );
	void SetLevel(int lev);
	void ProcessState();
	bool CheckHitPlayer(int index = 0);
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();
};

#endif