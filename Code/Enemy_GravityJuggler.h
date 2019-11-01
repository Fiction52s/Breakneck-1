#ifndef __ENEMY_GRAVITYJUGGLER_H__
#define __ENEMY_GRAVITYJUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct GravityJuggler : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_POP,
		S_JUGGLE,
		S_RETURN,
		S_Count
	};

	GravityJuggler(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, int p_level);
	void HandleEntrant(QuadTreeEntrant *qte);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void HandleNoHealth();
	CollisionBox &GetEnemyHitbox();
	void Move();
	void Return();
	void Pop();
	void PopThrow();

	void Throw(double a, double strength);
	void Throw(V2d vel);


	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];

	ComboObject *comboObj;

	V2d origPos;

	V2d velocity;

	bool reversedGrav;

	double gravFactor;
	V2d gDir;
	double maxFallSpeed;

	int hitLimit;
	int currHits;

	sf::Sprite sprite;
	Tileset *ts;

	bool reversed;

	int juggleReps;
	int currJuggle;
};

#endif