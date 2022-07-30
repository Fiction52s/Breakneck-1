#ifndef __ENEMY_WIDOW_H__
#define __ENEMY_WIDOW_H__

#include "Enemy.h"
#include "Bullet.h"

struct Widow : Enemy, RayCastHandler, SurfaceMoverHandler,
	LauncherEnemy
{
	enum Action
	{
		IDLE,

		MOVE,
		WAIT,
		JUMP,
		ATTACK,
		LAND,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};
	MyData data;

	Tileset *ts;

	sf::CircleShape ccwCircle;
	sf::CircleShape cwCircle;

	double gravity;

	V2d cwPreview;
	V2d ccwPreview;
	double distToCheck;

	double attentionRadius;

	double maxGroundSpeed;
	double maxFallSpeed;

	

	Widow(ActorParams *ap);
	~Widow();

	void CheckDist(double dist);
	bool TryMove();
	void ActionEnded();
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	void ResetEnemy();
	//void UpdateEnemyPhysics();
	void FrameIncrement();
	void HitTerrain(double &q);
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