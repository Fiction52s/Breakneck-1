#ifndef __ENEMY_EXPLODINGBARREL_H__
#define __ENEMY_EXPLODINGBARREL_H__

#include "Enemy.h"

struct ComboObject;

struct ExplodingBarrel : Enemy
{
	enum Action
	{
		S_IDLE,
		S_EXPLODE,
		S_Count
	};

	void SetLevel(int lev);

	ExplodingBarrel(ActorParams * ap);
	~ExplodingBarrel();

	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();

	sf::CircleShape testCircle;
	Tileset *ts;
	CollisionBody explosion;
};

#endif