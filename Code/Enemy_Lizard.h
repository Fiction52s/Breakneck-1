#ifndef __ENEMY_LIZARD_H__
#define __ENEMY_LIZARD_H__

#include "Enemy.h"
#include "Bullet.h"

struct Lizard : Enemy, GroundMoverHandler,
	LauncherEnemy
{
	enum Action
	{
		IDLE,
		RUN,
		//JUMPSQUAT,
		JUMP,
		LAND,
		SHOCK,
		Count
	};

	struct MyData : StoredEnemyData
	{
		int fireWaitCounter;
	};
	MyData data;

	int fireWaitDuration;
	bool bulletClockwise;

	Tileset *ts_bulletExplode;
	Tileset *ts;
	double gravity;
	double maxGroundSpeed;
	double maxFallSpeed;
	double runAccel;
	double runDecel;

	Lizard(ActorParams *ap);
	void FrameIncrement();
	void HandleNoHealth();
	void ActionEnded();
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();
	void SetLevel(int lev);
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();
	void Shock();

	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		sf::Vector2<double> &pos);
	void BulletHitPlayer(
		int playerIndex,
		BasicBullet *b,
		int hitResult);
	void UpdateBullet(BasicBullet *b);
	void FireResponse(BasicBullet *b);
	void DirectKill();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif