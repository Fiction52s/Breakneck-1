#ifndef __ENEMY_HAND_H__
#define __ENEMY_HAND_H__

#include "Enemy.h"
#include "Bullet.h"

struct Hand : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		NEUTRAL,
		APPEAR,
		A_Count
	};

	Tileset *ts;
	double maxSpeed;
	double accel;
	
	bool isRight;

	Hand( bool right );

	void Appear(V2d &pos);

	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();

	void HitTerrainAerial(Edge *, double);
};

#endif