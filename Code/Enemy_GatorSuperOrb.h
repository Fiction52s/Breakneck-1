#ifndef __ENEMY_GATORSUPERORB_H__
#define __ENEMY_GATORSUPERORB_H__

#include "Enemy.h"
#include <vector>
#include "EnemyMover.h"

struct GatorSuperOrb;

struct GatorSuperOrbPool
{
	GatorSuperOrbPool();
	~GatorSuperOrbPool();
	void Reset();
	GatorSuperOrb * Throw(V2d &pos, V2d &dir);
	void Draw(sf::RenderTarget *target);
	bool CanThrow();
	void ReturnToGator( V2d &pos );
	bool IsIdle();
	void SetEnemyIDAndAddToAllEnemiesVec();
	std::vector<GatorSuperOrb*> bulletVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numBullets;
};

struct GatorSuperOrb : Enemy
{
	enum Action
	{
		CHASING,
		CENTER,
		GROW,
		STASIS,
		RETURN_TO_GATOR,
		LAUNCH,
		DISSIPATE,
		A_Count
	};

	Tileset *ts;

	double flySpeed;
	double maxFlySpeed;
	double accel;
	V2d velocity;
	double distToTarget;

	sf::Vertex *quad;
	EnemyMover enemyMover;
	//for node grow
	double currRadius;
	double startRadius;
	double maxRadius;
	V2d targetPos;

	GatorSuperOrb(sf::Vertex *quad,
		GatorSuperOrbPool *pool);
	void ReturnToGator( V2d &pos );
	void UpdateEnemyPhysics();
	void SetPos(V2d &pos);
	void Die();
	bool IsIdle();
	void Launch(V2d &pos, double extraHeight, double speed);
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