#ifndef __ENEMY_EXPLODINGBARREL_H__
#define __ENEMY_EXPLODINGBARREL_H__

#include "Enemy.h"

struct ComboObject;

struct ExplodingBarrel : Enemy
{
	enum Action
	{
		S_IDLE,
		S_TINYCHARGE,
		S_CHARGE,
		S_EXPLODE,
		S_Count
	};

	sf::CircleShape testCircle;
	Tileset *ts;
	CollisionBody explosion;

	

	ExplodingBarrel(ActorParams * ap);
	~ExplodingBarrel();

	void SetLevel(int lev);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();

	
};

#endif