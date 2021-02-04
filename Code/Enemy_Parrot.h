#ifndef __ENEMY_PARROT_H__
#define __ENEMY_PARROT_H__

#include "Enemy.h"
#include "Bullet.h"

struct Parrot : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		FLY,
		A_Count
	};

	double attentionRadius;
	double ignoreRadius;

	Tileset *ts;
	V2d velocity;
	double maxSpeed;
	double accel;

	int fireCounter;



	Parrot(ActorParams *ap);

	void HandleNoHealth();
	void FrameIncrement();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();

	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(int playerIndex,
		BasicBullet *b, int hitResult);
	void DirectKill();
};

#endif