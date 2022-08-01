#ifndef __ENEMY_PREDICTTURRET_H__
#define __ENEMY_PREDICTTURRET_H__

#include "Enemy.h"
#include "Bullet.h"

struct PredictTurret : Enemy, LauncherEnemy
{
	enum Action
	{
		WAIT,
		ATTACK,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;
	Tileset *ts_bulletExplode;

	sf::CircleShape testCircle;
	int framesWait;
	int firingCounter;

	int animationFactor;
	double bulletSpeed;

	int futureFrames;

	PredictTurret(ActorParams *ap);
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
		int hitResult);
	void UpdateBullet(BasicBullet *b);
	void ResetEnemy();
	void FireResponse(BasicBullet *b);
	int GetNumSimulationFramesRequired();
	void UpdatePreFrameCalculations();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif