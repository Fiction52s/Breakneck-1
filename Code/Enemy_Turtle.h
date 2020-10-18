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

	Tileset *ts_bulletExplode;
	int bulletSpeed;

	int fireCounter;

	Tileset *ts;

	V2d playerTrackPos;

	Turtle(ActorParams *ap);
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(int playerIndex, 
		BasicBullet *b, int hitResult);
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void DirectKill();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	
	void SetLevel(int lev);
	
	void UpdateSprite();
	void ResetEnemy();
};

#endif