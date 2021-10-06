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
		LASER,
		WALK,
		WIRETHROW,
		WIRE_IDLE,
		WIREPULL,
		A_Count
	};

	//Tileset *ts_walk;
	Tileset *ts;

	int moveFrames;
	int waitFrames;

	EnemyMover enemyMover;

	int targetPlayerIndex;

	V2d targetPos;
	int framesToArrive;

	sf::Vertex wireQuad[4];

	double extraHeight;
	double wireThrowSpeed;

	V2d currWirePos;
	V2d wireAnchor;
	int framesThrowingWire;

	SequenceSkeleton(ActorParams *ap);
	void Wait();

	void UpdateWire();
	void Walk(V2d &pos);
	void Laser();
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