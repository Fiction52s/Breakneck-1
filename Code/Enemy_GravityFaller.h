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

	GravityFaller(GameSession *owner,
		bool hasMonitor,
		Edge *ground, double quantity,
		int fallerVariation );

	int actionLength[A_Count];
	int animFactor[A_Count];
	void UpdateEnemyPhysics();
	void FrameIncrement();
	Tileset *ts;

	int fallFrames;

	Action action;
	bool facingRight;
	double gravity;
	sf::Vector2<double> velocity;
	double gravityFactor;
	SurfaceMover *mover;
	int chargeLength;
	int chargeFrame;

	void ProcessState();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();

	void HitTerrainAerial(Edge *e, double quant);

	sf::Sprite sprite;
	Tileset *ts_walk;
	Tileset *ts_roll;

	double maxFallSpeed;

	Edge *startGround;
	double startQuant;
};

#endif