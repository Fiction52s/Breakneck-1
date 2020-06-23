#ifndef __ENEMY_GRAVITYFALLER_H__
#define __ENEMY_GRAVITYFALLER_H__

#include "Enemy.h"

struct GravityFaller : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		IDLE,
		DOWNCHARGE,
		FALLDOWN,
		UPCHARGE,
		FALLUP,
		REVERSEUPTODOWN,
		REVERSEDOWNTOUP,
		A_Count
	};

	GravityFaller(ActorParams *ap);

	void SetLevel(int lev);
	int actionLength[A_Count];
	int animFactor[A_Count];
	void UpdateEnemyPhysics();
	void FrameIncrement();
	Tileset *ts;

	int fallFrames;

	double gravity;
	sf::Vector2<double> velocity;
	double gravityFactor;
	int chargeLength;
	int chargeFrame;

	void ProcessState();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();

	void HitTerrainAerial(Edge *e, double quant);

	Tileset *ts_walk;
	Tileset *ts_roll;

	double maxFallSpeed;
};

#endif