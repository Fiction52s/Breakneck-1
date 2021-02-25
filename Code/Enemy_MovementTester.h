#ifndef __ENEMY_MOVEMENTTESTER_H__
#define __ENEMY_MOVEMENTTESTER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_BirdShuriken.h"
#include "Enemy_CoyoteBullet.h"
#include "Enemy_TigerGrindBullet.h"
#include "Enemy_GatorWaterOrb.h"
#include "Enemy_CoyoteShockwave.h"
#include "Enemy_TigerSpinTurret.h"
#include "EnemyMover.h"


struct MovementTester : Enemy, RayCastHandler
{
	enum Action
	{
		MOVE,
		WAIT,
		A_Count
	};

	EnemyMover enemyMover;

	int testCounter;

	BirdShurikenPool shurPool;
	CoyoteBulletPool coyBulletPool;
	TigerGrindBulletPool tigerBulletPool;
	GatorWaterOrbPool gatorOrbPool;
	CoyoteShockwavePool coyShockPool;
	//TigerSpinTurretPool tigerSpinTurretPool;

	double approachStartDist;
	CubicBezier approachBez;
	
	V2d *chaseTarget;
	V2d chaseOffset;
	double chaseMaxVel;
	double chaseAccel;

	V2d velocity;
	double accel;
	double maxSpeed;
	//SpaceMover *sMover;
	int moveFrames;
	int startMoveFrames;
	V2d startMovePlayerPos;
	int waitFrames;
	int maxWaitFrames;
	sf::CircleShape predictCircle;

	bool predict;

	int targetPlayerIndex;

	LineMovement *move;
	MovementSequence ms;

	MovementSequence curveMovement;
	//CubicMovement *curve;
	QuadraticMovement *qCurve;

	int testCheck;

	HitboxInfo hitboxInfos[A_Count];

	V2d targetPos;

	MovementTester(ActorParams *ap);
	~MovementTester();

	sf::FloatRect GetAABB();

	void DebugDraw(sf::RenderTarget *target);
	void CalcMovement();
	void UpdatePreFrameCalculations();
	void ProcessState();
	void CalcTargetAfterHit();
	void CalcPlayerFuturePos(int frames);
	void UpdateHitboxes();

	void EnemyDraw(sf::RenderTarget *target);
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);

	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	void SetHitboxInfo(int a);

	sf::CircleShape myCircle;


};

#endif