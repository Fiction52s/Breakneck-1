#ifndef __ENEMY_SEQUENCESKELETON_H__
#define __ENEMY_SEQUENCESKELETON_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "EnemyMover.h"

struct GameSession;
struct SequenceBird;

struct SequenceSkeleton : Enemy
{
	enum Action
	{
		IDLE,
		CHARGELASER,
		LASER,
		LASER_IDLE,
		WALK,
		JUMPSQUAT,
		HOP,
		LAND,
		WIRETHROW,
		WIRE_PEACE,
		WIRE_IDLE,
		WIREPULL,
		RIDE_BIRD,
		MIND_CONTROL,
		MIND_CONTROL_IDLE,
		A_Count
	};

	//Tileset *ts_walk;
	Tileset *ts;
	Tileset *ts_laser;
	Tileset *ts_bullet;
	Tileset *ts_walk;
	Tileset *ts_hop;
	Tileset *ts_wireAway;
	int moveFrames;
	int waitFrames;

	EnemyMover enemyMover;

	int targetPlayerIndex;

	V2d targetPos;
	int framesToArrive;

	V2d hopTarget;
	double hopSpeed;
	double hopExtraHeight;

	V2d laserPos;
	double laserSpeed;
	int laserFrame;
	int laserAnimFactor;
	int laserAnimFrames;
	V2d laserVel;

	

	sf::Vertex wireQuad[4];
	sf::Vertex laserQuad[4];

	double extraHeight;
	double wireThrowSpeed;

	V2d currWirePos;
	V2d wireAnchor;
	V2d wireInitialThrowOrigin;
	bool wireMoving;


	SequenceBird *seqBird;
	V2d offsetFromBird;
	//int framesThrowingLaser;

	SequenceSkeleton(ActorParams *ap);
	void Wait();

	V2d GetWireOrigin();
	void RideBird(SequenceBird *seqBird);
	void Hop( V2d &pos, double p_hopSpeed, double p_hopExtraHeight );
	void Idle();
	void ChargeLaser();
	void UpdateWire();
	void Walk(V2d &pos);
	void Laser( V2d &pos );
	void WireThrow(V2d &pos);
	void WirePull();
	void UpdateWireQuad();
	void MindControl();

	void ProcessState();
	void DebugDraw(sf::RenderTarget *target);

	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void DrawWire(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target) {}
	virtual bool SpawnableByCamera() { return false; }
};

#endif