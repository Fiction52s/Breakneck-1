#ifndef __ENEMY_SEQUENCESKELETON_H__
#define __ENEMY_SEQUENCESKELETON_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "EnemyMover.h"

struct GameSession;

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
		WIRE_IDLE,
		WIREPULL,
		A_Count
	};

	//Tileset *ts_walk;
	Tileset *ts;
	Tileset *ts_laser;
	Tileset *ts_bullet;
	Tileset *ts_walk;
	Tileset *ts_hop;
	int moveFrames;
	int waitFrames;

	EnemyMover enemyMover;

	int targetPlayerIndex;

	V2d targetPos;
	int framesToArrive;

	V2d hopTarget;

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
	int framesThrowingWire;

	//int framesThrowingLaser;

	SequenceSkeleton(ActorParams *ap);
	void Wait();

	void HopDown( V2d &pos );
	void Idle();
	void ChargeLaser();
	void UpdateWire();
	void Walk(V2d &pos);
	void Laser( V2d &pos );
	void WireThrow(V2d &pos);
	void WirePull();
	void UpdateWireQuad();

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