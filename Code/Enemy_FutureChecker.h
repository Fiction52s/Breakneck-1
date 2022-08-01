#ifndef __ENEMY_FUTURECHECKER_H__
#define __ENEMY_FUTURECHECKER_H__

#include "Enemy.h"
#include "Bullet.h"

struct FutureChecker : Enemy
{
	enum Action
	{
		NEUTRAL,
		FIRE,
		INVISIBLE,
		FADEIN,
		FADEOUT,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
		V2d velocity;
		V2d playerTrackPos;
	};
	MyData data;

	Tileset *ts_bulletExplode;
	int bulletSpeed;
	const static int predictFrames;
	Tileset *ts;
	double accel;
	double maxSpeed;

	FutureChecker(ActorParams *ap);
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	int GetNumSimulationFramesRequired();
	void SetLevel(int lev);
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void UpdatePreFrameCalculations();

	void UpdateSprite();
	void ResetEnemy();
};

#endif