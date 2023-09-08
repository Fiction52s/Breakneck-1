#ifndef __ENEMY_TURTLE_H__
#define __ENEMY_TURTLE_H__

#include "Enemy.h"
#include "Bullet.h"

struct Turtle : Enemy, LauncherEnemy
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
		V2d playerTrackPos;
	};
	MyData data;

	int bulletSpeed;
	Tileset *ts;
	sf::CircleShape trackTestCircle;

	Turtle(ActorParams *ap);
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