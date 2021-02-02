#ifndef __ENEMY_LASERJAYS_H__
#define __ENEMY_LASERJAYS_H__

#include "Enemy.h"
#include "Bullet.h"

struct LaserJays : Enemy
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
	CollisionBody laserBody;

	bool secondary;
	LaserJays *otherJay;
	ActorParams *otherParams;

	sf::Vertex laserQuad[4];

	LaserJays(ActorParams *ap);
	LaserJays(ActorParams *ap, bool secondary);
	~LaserJays();

	void Construct(ActorParams *ap);

	void AddToGame();
	void TryCreateSecondary(ActorParams *ap);
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