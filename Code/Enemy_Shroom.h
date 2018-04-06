#ifndef _ENEMY_SHROOM__H_
#define _ENEMY_SHROOM__H_

#include "Enemy.h"

struct ShroomJelly;

struct Shroom : Enemy
{
	enum Action
	{
		LATENT,
		HITTING,
	};

	Action action;
	int actionLength[HITTING + 1];
	int animFactor[HITTING + 1];

	Shroom(GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void HandleNoHealth();
	void ResetEnemy();
	void ProcessState();

	sf::Sprite sprite;
	Tileset *ts;

	Edge *ground;
	double edgeQuantity;

	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	HitboxInfo *hitboxInfo;

	double angle;

	sf::Vector2<double> gn;

	ShroomJelly *jelly;
};

struct ShroomJelly : Enemy
{
	enum Action
	{
		WAIT,
		APPEARING,
		RISING,
		DROOPING,
		DISSIPATING
	};

	Action action;
	int actionLength[DISSIPATING + 1];
	int animFactor[DISSIPATING + 1];

	ShroomJelly(GameSession *owner, V2d &pos );
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void ProcessState();
	void UpdateEnemyPhysics();
	HitboxInfo *IsHit(Actor *player);

	sf::Sprite sprite;
	Tileset *ts;

	int cycleLimit;
	int currentCycle;

	V2d velocity;

	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	HitboxInfo *hitboxInfo;

	CubicBezier risingBez;
	CubicBezier fallingBez;

	double angle;
};

#endif