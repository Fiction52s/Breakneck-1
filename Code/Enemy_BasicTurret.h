#ifndef __ENEMY_BASICTURRET_H__
#define __ENEMY_BASICTURRET_H__

#include "Enemy.h"

struct Shield;


struct BasicTurret : Enemy, LauncherEnemy
{
	enum Action
	{
		WAIT,
		ATTACK,
	};

	BasicTurret(GameSession *owner, bool hasMonitor, Edge *ground, double quantity,
		double bulletSpeed,
		int framesWait);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void Setup();
	Tileset *ts_bulletExplode;
	int frameTestCounter;
	void UpdatePreLauncherPhysics();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
	sf::SoundBuffer *fireSound;
	CollisionBox prelimBox[3];

	//bool playerPrelimHit[4];

	void ResetEnemy();

	sf::Sprite sprite;
	sf::Sprite auraSprite;
	Tileset *ts;
	Tileset *ts_aura;
	Action action;

	const static int maxBullets = 16;

	void DirectKill();

	Shield *testShield;

	HitboxInfo *bulletHitboxInfo;

	int framesWait;
	int firingCounter;
	Edge *ground;
	double edgeQuantity;

	void SetupPreCollision();
	CollisionBody *prelimBody;

	double angle;

	int animationFactor;
	sf::Vector2<double> gn;
	double bulletSpeed;
};

#endif