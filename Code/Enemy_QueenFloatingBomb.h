#ifndef __ENEMY_QUEENFLOATINGBOMB_H__
#define __ENEMY_QUEENFLOATINGBOMB_H__

#include "Enemy.h"
#include "ObjectPool.h"

struct QueenFloatingBomb : Enemy, SurfaceMoverHandler, PoolMember
{
	enum Action
	{
		FLOATING,
		EXPLODING,
		COMBO,
		Count
	};

	Action action;
	QueenFloatingBomb(ActorParams *ap, ObjectPool *myPool, int index);
	~QueenFloatingBomb();
	int actionLength[Count];
	int animFactor[Count];
	SurfaceMover *mover;
	Tileset *ts;
	sf::Sprite sprite;
	ObjectPool *myPool;
	void Init(V2d pos, V2d vel);
	void ProcessState();
	void HandleNoHealth();
	void HitTerrainAerial(Edge *, double);
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void ProcessHit();
};

#endif