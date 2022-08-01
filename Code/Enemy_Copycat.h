#ifndef __ENEMY_COPYCAT_H__
#define __ENEMY_COPYCAT_H__

#include "Enemy.h"
#include "Bullet.h"
struct Actor;

struct Copycat : Enemy, GroundMoverHandler,
	LauncherEnemy
{
	enum Action
	{
		IDLE,
		STAND,
		RUN,
		JUMP,
		DOUBLEJUMP,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
		bool hasDoubleJump;
	};
	MyData data;

	double attentionRadius;
	double ignoreRadius;
	int preChargeFrames;
	int preChargeLimit;
	double boostSpeed;

	double airAccel;
	double runAccel;
	double runDecel;
	double turnaroundDist;
	double boostPastDist;

	Tileset *ts;

	double maxGroundSpeed;
	double maxFallSpeed;

	Actor *player;

	Copycat(ActorParams *ap);
	bool HoldingLeft();
	bool HoldingRight();
	void RunMovement();
	bool TryJump();
	bool TryDoubleJump();
	void ActionEnded();
	void ProcessState();
	void SetLevel(int lev);
	void UpdateEnemyPhysics();
	void HandleNoHealth();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void Jump(double strengthx,
		double strengthy);
	bool StartRoll();
	void FinishedRoll();
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(int playerIndex,
		BasicBullet *b, int hitResult);
	void DirectKill();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};


#endif