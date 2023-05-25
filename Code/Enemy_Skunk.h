#ifndef __ENEMY_SKUNK_H__
#define __ENEMY_SKUNK_H__

#include "Enemy.h"

struct Skunk : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
		WALK,
		HOP,
		CHARGE,
		EXPLODE,
		LAND,
		Count
	};

	struct MyData : StoredEnemyData
	{
		CollisionBody explosion;
		int refreshFrame;
	};
	MyData data;

	int maxRefreshFrames;

	Tileset *ts;
	double gravity;
	double maxGroundSpeed;
	double maxFallSpeed;
	double runAccel;
	double runDecel;
	double attentionRadius;
	double ignoreRadius;
	
	sf::CircleShape testCircle;
	double explosionRadius;
	HitboxInfo explosionHitboxInfo;

	Skunk(ActorParams *ap);
	void IHitPlayer( int index = 0);
	void ProcessHit();
	void HandleNoHealth();
	void Hop();
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
	void FrameIncrement();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

};

#endif