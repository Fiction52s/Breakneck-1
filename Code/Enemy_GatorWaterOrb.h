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
	GatorWaterOrb * Throw(V2d &pos, V2d &dir);
	void Draw(sf::RenderTarget *target);
	std::vector<GatorWaterOrb*> bulletVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numBullets;
};

struct GatorWaterOrb : Enemy
{
	enum Action
	{
		UNDODGEABLE,
		FLYING,
		A_Count
	};

	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	double flySpeed;
	double maxFlySpeed;
	double accel;
	int framesToLive;
	int origFramesToLive;

	V2d velocity;

	sf::Vertex *quad;

	double distToTarget;

	GatorWaterOrb(sf::Vertex *quad,
		GatorWaterOrbPool *pool);
	void UpdateEnemyPhysics();
	void Throw(V2d &pos, V2d &dir);
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