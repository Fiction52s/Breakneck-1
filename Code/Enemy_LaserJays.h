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

	struct MyData : StoredEnemyData
	{
		V2d velocity;
	};
	MyData data;

	double activatePulseRadius;

	Tileset *ts;
	
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
	void UpdateOnPlacement(ActorParams *ap);
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif