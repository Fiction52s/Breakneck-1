#ifndef __ENEMY_TRAILER_H__
#define __ENEMY_TRAILER_H__

#include "Enemy.h"
#include "Bullet.h"

struct Trailer : Enemy
{
	enum Action
	{
		IDLE,
		TRAIL,
		PULSE,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		MovementSequence trailSeq;
		V2d velocity;
	};
	MyData data;

	double attentionRadius;
	double ignoreRadius;

	Tileset *ts;
	
	double maxSpeed;
	double accel;
	double pulseRadius;
	CollisionBody pulseBody;
	sf::CircleShape testCircle;

	
	LineMovement *trailMove;

	Trailer(ActorParams *ap);
	void StartTrail();

	void ApproachMovement();
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
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