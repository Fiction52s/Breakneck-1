#ifndef __ENEMY_GATORWATERORB_H__
#define __ENEMY_GATORWATERORB_H__

#include "Enemy.h"
#include <vector>

struct GatorWaterOrb;

struct GatorWaterOrbPool
{
	enum Action
	{
		NORMAL,
		CIRCLE,
	};

	GatorWaterOrbPool();
	~GatorWaterOrbPool();
	void Reset();
	GatorWaterOrb * Throw(V2d &pos, V2d &dir,
		int orbType );
	void Draw(sf::RenderTarget *target);
	void Redirect(V2d &vel);
	bool RedirectOldestAtPlayer(Actor *p, double speed );
	bool RedirectOldest(V2d &vel);
	int GetNumActive();
	int GetNumGrowingOrbs();
	void GroupChase(V2d *target,
		double p_chaseAccel, 
		double p_chaseMaxSpeed);
	void Chase(V2d *target,
		double p_chaseAccel,
		double p_chaseMaxSpeed );
	void CircleChase(V2d *target, double p_chaseAccel,
		double p_chaseMaxSpeed);
	void StopChase();
	void StopCircleChase();
	void CreateCircle(V2d &pos, int numOrbs,
		double radius, double orbRadius,
		double startAngle,
		int orbType = 0);
	void SetCircleTimeToLive(int frames);
	void SetCircleVelocity(V2d &vel);
	GatorWaterOrb *GetOldest();
	void EndCircle();
	bool CanThrow();
	void RotateCircle(double rotSpeed,
		double rotAccel = 0 ,
		double maxRotSpeed = 0 );

	void ExpandCircle(double expandSpeed,
		double accel = 0, double maxExpandSpeed = 0 );
	void ExpandCircleToRadius( double endSize, double expandSpeed,
		double accel = 0, double maxExpandSpeed = 0);
	void ChangeAllCircleOrbsRadiusOverTime(double orbGrowSpeed,
		double goalRadius );
	void SetCircleFollowPos(V2d *followTarget);
	void StopCircleFollow();
	bool IsChangingSize();

	void Update();
	
	double orbGrowSpeed;

	double circleRotateSpeed;
	double circleRotateAccel;
	double circleRotateMaxSpeed;
	V2d circleVel;
	V2d circleCenter;
	double circleRadius;
	bool useCircleGoalRadius;
	double circleGoalRadius;
	double circleExpandSpeed;
	double circleExpandAccel;
	double circleExpandMaxSpeed;
	
	V2d *followTarget;
	V2d followOffset;

	std::vector<GatorWaterOrb*> bulletVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numBullets;
	V2d *chaseTarget;
	double chaseAccel;
	double chaseMaxSpeed;
	int action;
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
		FLOATING,
		REDIRECT,
		CHASE,
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

	sf::Vertex *quad;

	//for node grow
	double currRadius;
	double startRadius;
	double maxRadius;
	V2d targetPos;

	GatorWaterOrbPool *pool;

	double chaseAccel;
	double maxChaseSpeed;
	V2d startChasingPos;

	QuadraticMovement *quadraticMove;
	MovementSequence quadraticMoveSeq;

	double goalRadius;
	double growthFactor;

	GatorWaterOrb(sf::Vertex *quad,
		GatorWaterOrbPool *pool);
	void UpdateEnemyPhysics();
	void Die();
	void Throw(V2d &pos, V2d &dir, int orbType );
	void CreateForCircle(V2d &pos, double orbRadius,
		int orbType );
	void Redirect(V2d &vel);
	void SetLevel(int lev);
	void GroupChase(double p_chaseAccel, 
		double p_chaseMaxSpeed);
	void Chase(double p_chaseAccel, 
		double p_chaseMaxSpeed);
	void ProcessState();
	bool CheckHitPlayer(int index = 0);
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void SetRadius(double rad);
	void FrameIncrement();
	void ChangeRadiusOverTime(double growthFactor,
		double endSize);
};

#endif