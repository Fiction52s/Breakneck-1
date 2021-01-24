#ifndef __ENEMY_FIREFLY_H__
#define __ENEMY_FIREFLY_H__

#include "Enemy.h"
#include "Bullet.h"

struct Firefly : Enemy
{
	enum Action
	{
		NEUTRAL,
		APPROACH,
		CHARGE,
		PULSE,
		RECOVER,
		A_Count
	};

	double attentionRadius;
	double ignoreRadius;
	double activatePulseRadius;

	Tileset *ts;
	V2d velocity;
	double maxSpeed;
	double accel;
	double pulseRadius;
	CollisionBody pulseBody;
	sf::CircleShape testCircle;

	Firefly(ActorParams *ap);

	void ApproachMovement();
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();
};

#endif