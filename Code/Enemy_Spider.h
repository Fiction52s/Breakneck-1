#ifndef __ENEMY_SPIDER_H__
#define __ENEMY_SPIDER_H__

#include "Enemy.h"

struct Spider : Enemy, RayCastHandler, SurfaceMoverHandler
{
	enum Action
	{
		MOVE,
		JUMP,
		ATTACK,
		LAND,
		A_Count
	};

	Spider(ActorParams *ap);
	~Spider();
	void ActionEnded();
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void UpdatePostPhysics();
	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();
	void CheckClosest(Edge * e,
		V2d &playerPos,
		bool right,
		double cutoffQuant);
	void SetClosestLeft();
	void SetClosestRight();
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	V2d closestPos;
	Edge *closestEdge;
	double closestQuant;
	bool closestClockwiseFromCurrent;

	Tileset *ts;

	bool canSeePlayer;

	int framesLaseringPlayer;
	int laserLevel;

	double laserAngle;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	HitboxInfo *laserInfo0;
	HitboxInfo *laserInfo1;
	HitboxInfo *laserInfo2;
	HitboxInfo *laserInfo3;
	int laserCounter;

	V2d gravity;

	int attackFrame;
	int attackMult;

	double maxGroundSpeed;
	double maxFallSpeed;
};

#endif