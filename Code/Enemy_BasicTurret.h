#ifndef __ENEMY_BASICTURRET_H__
#define __ENEMY_BASICTURRET_H__

#include "Enemy.h"

struct BasicTurret : Enemy, LauncherEnemy
{
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

	CollisionBox prelimBox[3];
	bool playerPrelimHit[4];

	void ResetEnemy();

	sf::Sprite sprite;
	Tileset *ts;

	const static int maxBullets = 16;

	void DirectKill();

	HitboxInfo *bulletHitboxInfo;

	int framesWait;
	int firingCounter;
	Edge *ground;
	double edgeQuantity;

	void SetupPreCollision();
	CollisionBody *prelimBody;
	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	HitboxInfo *hitboxInfo;

	double angle;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	int animationFactor;
	sf::Vector2<double> gn;
	double bulletSpeed;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
};

#endif