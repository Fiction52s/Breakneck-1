#ifndef __ENEMY_GHOST_H__
#define __ENEMY_GHOST_H__

#include "Enemy.h"

struct Ghost : Enemy
{
	enum Action
	{
		WAKEUP,
		APPROACH,
		BITE,
		EXPLODE,
		RETURN,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int awakeFrames;
		double latchStartAngle;
		V2d basePos;
		bool latchedOn;
		int totalFrame;
		V2d offsetPlayer;
		V2d origOffset;
	};
	MyData data;

	double detectionRadius;
	
	int awakeCap;

	double approachSpeed;

	Tileset *ts;

	Ghost(ActorParams *ap);
	void SetLevel(int lev);
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void Bite();
	void HandleNoHealth();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif