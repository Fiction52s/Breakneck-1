#ifndef __ENEMY_DRAGON_H__
#define __ENEMY_DRAGON_H__

#include "Enemy.h"
#include "Bullet.h"

struct Dragon : Enemy
{
	enum Action
	{
		NEUTRAL,
		FLY,
		RUSH,
		A_Count
	};

	const static int MAX_PAST_POSITIONS = 60;

	struct MyData : StoredEnemyData
	{
		int recoverFrame;
		V2d velocity;
		V2d pastPositions[MAX_PAST_POSITIONS];
		int numPastPositions;
	};

	const static int NUM_SEGMENTS = 60;//6;
	sf::Vertex segmentQuads[NUM_SEGMENTS * 4];

	MyData data;

	Tileset *ts;

	V2d maxSpeed;
	V2d accel;
	double rushSpeed;

	int recoverDuration;
	V2d testOffsetDir;

	Dragon(ActorParams *ap);

	void FrameIncrement();
	void AddPastPosition(V2d pos);
	void FlyMovement();
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif