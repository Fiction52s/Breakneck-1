#ifndef __ENEMY_QUEENFLOATINGBOMB_H__
#define __ENEMY_QUEENFLOATINGBOMB_H__

#include "Enemy.h"

struct QueenFloatingBomb : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		FLOATING,
		EXPLODING,
		COMBO,
		A_Count
	};

	Action action;
	QueenFloatingBomb(/*ActorParams *ap*/);
	~QueenFloatingBomb();

	Tileset *ts;
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
	void ProcessHit();
};

#endif