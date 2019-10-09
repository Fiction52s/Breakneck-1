#ifndef __ENEMY_JUGGLER_H__
#define __ENEMY_JUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct Juggler : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_POP,
		S_JUGGLE,
		S_RETURN,
		S_EXPLODE,
		S_Count
	};

	Juggler(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, int p_level);
	//Comboer(GameSession *owner, std::ifstream &is);

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

	ComboObject *comboObj;

	V2d origPos;

	V2d velocity;

	double vertStrength;
	double horizStrength;
	double gravFactor;

	int hitLimit;
	int currHits;

	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];

	sf::Sprite sprite;
	Tileset *ts;

	int juggleReps;
	int currJuggle;
};

#endif