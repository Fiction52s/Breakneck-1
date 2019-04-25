#ifndef __ENEMY_CURVETURRET_H__
#define __ENEMY_CURVETURRET_H__

#include "Enemy.h"

struct CurveTurret : Enemy, LauncherEnemy
{
	enum Action
	{
		WAIT,
		ATTACK,
	};

	Action action;
	CurveTurret(GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity,
		double bulletSpeed,
		int framesWait,
		sf::Vector2i &gravFactor,
		bool relativeGrav);
	void EnemyDraw(sf::RenderTarget *target);
	void ProcessState();
	void Setup();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
	void ResetEnemy();

	sf::Sprite sprite;
	Tileset *ts;
	Tileset *ts_bulletExplode;
	const static int maxBullets = 16;

	int framesWait;
	int firingCounter;
	int realWait;
	Edge *ground;
	double edgeQuantity;

	Shield *shield;

	double angle;

	sf::Vector2<double> gravity;

	int animationFactor;
	sf::Vector2<double> gn;
	double bulletSpeed;
};

#endif