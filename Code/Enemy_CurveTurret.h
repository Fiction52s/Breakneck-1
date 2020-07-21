#ifndef __ENEMY_CURVETURRET_H__
#define __ENEMY_CURVETURRET_H__

#include "Enemy.h"
#include "Bullet.h"

struct CurveTurret : Enemy, LauncherEnemy
{
	enum Action
	{
		WAIT,
		ATTACK,
		Count
	};

	CurveTurret(ActorParams *ap);
	void EnemyDraw(sf::RenderTarget *target);
	void ProcessState();
	void Setup();
	void SetLevel(int lev);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		sf::Vector2<double> &pos);
	void BulletHitPlayer(
		int playerIndex,
		BasicBullet *b,
		int hitResult );
	void UpdateBullet(BasicBullet *b);
	void ResetEnemy();
	void FireResponse(BasicBullet *b);

	Tileset *ts;
	Tileset *ts_bulletExplode;
	const static int maxBullets = 16;

	int framesWait;
	int firingCounter;
	int realWait;

	double spins[16];
	int startBulletIndex;

	Shield *shield;

	double angle;

	sf::Vector2<double> gravity;

	int animationFactor;
	double bulletSpeed;

	int turnFactor;
};

#endif