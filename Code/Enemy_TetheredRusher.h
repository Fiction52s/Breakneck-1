#ifndef __ENEMY_TETHEREDRUSHER_H__
#define __ENEMY_TETHEREDRUSHER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"

struct TetheredRusher : Enemy
{
	enum Action
	{
		NEUTRAL,
		RUSH,
		RECOVER,
		A_Count
	};

	double attentionRadius;
	double ignoreRadius;
	double chainRadius;

	Tileset *ts;
	V2d velocity;
	double maxSpeed;
	double accel;
	sf::CircleShape testCircle;
	sf::CircleShape anchorCircle;

	MovementSequence ms;
	LineMovement *attackMovement;
	LineMovement *retreatMovement;
	V2d anchorPos;
	double anchorRadius;


	const static int NUM_SEGMENTS = 6;
	sf::Vertex segmentQuads[NUM_SEGMENTS * 4];

	TetheredRusher(ActorParams *ap);

	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void UpdateHitboxes();
	bool IsSlowed(int index = 0);
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();
};

#endif