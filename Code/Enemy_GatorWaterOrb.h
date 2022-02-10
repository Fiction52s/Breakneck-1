#ifndef __ENEMY_GATORWATERORB_H__
#define __ENEMY_GATORWATERORB_H__

#include "Enemy.h"
#include <vector>

struct GatorWaterOrb;

struct GatorWaterOrbPool
{
	/*enum Action
	{
		NORMAL,
		CIRCLE,
	};*/

	GatorWaterOrbPool();
	~GatorWaterOrbPool();
	void Reset();
	GatorWaterOrb * Throw(V2d &pos, V2d &dir,
		int orbType );
	void Draw(sf::RenderTarget *target);
	void Redirect(V2d &vel);
	bool RedirectOldestAtPlayer(Actor *p, double speed );
	bool RedirectOldest(V2d &vel);
	int GetNumGrowingOrbs();
	void GroupChase(V2d *target);
	void CreateCircle(V2d &pos, int numOrbs,
		double radius, double orbRadius,
		double startAngle );
	void StopChase();
	GatorWaterOrb *GetOldest();
	bool CanThrow();
	void RotateCircle(double rotSpeed,
		double rotAccel = 0 ,
		double maxRotSpeed = 0 );
	void ExpandCircle(double expandSpeed,
		double accel = 0, double maxExpandSpeed = 0 );
	void Update();
	
	double circleRotateSpeed;
	double circleRotateAccel;
	double circleRotateMaxSpeed;
	V2d circleVel;
	V2d circleCenter;
	double circleRadius;
	double circleExpandSpeed;
	double circleExpandAccel;
	double circleExpandMaxSpeed;

	std::vector<GatorWaterOrb*> bulletVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numBullets;
	V2d *chaseTarget;
	V2d GetActiveCenter();
};

struct GatorWaterOrb : Enemy
{
	enum OrbType
	{
		NODE_GROW_HIT,
		NODE_GROW_SLOW,
	};

	enum Action
	{
		CIRCLE_APPEAR,
		FLYING,
		GROWING,
		REDIRECT,
		GROUP_CHASE,
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
	bool growing;

	sf::Vertex *quad;

	//for node grow
	double currRadius;
	double startRadius;
	double maxRadius;
	V2d targetPos;

	GatorWaterOrbPool *pool;

	double chaseAccel;
	double maxChaseVel;
	V2d startChasingPos;

	QuadraticMovement *quadraticMove;
	MovementSequence quadraticMoveSeq;

	GatorWaterOrb(sf::Vertex *quad,
		GatorWaterOrbPool *pool);
	void UpdateEnemyPhysics();
	void Die();
	void Throw(V2d &pos, V2d &dir, int orbType );
	void CreateForCircle(V2d &pos, double orbRadius );
	void Redirect(V2d &vel);
	void SetLevel(int lev);
	void GroupChase();
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