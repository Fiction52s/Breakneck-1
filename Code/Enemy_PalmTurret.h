#ifndef __ENEMY_PALMTURRET_H__
#define __ENEMY_PALMTURRET_H__

#include "Enemy.h"
#include "Bullet.h"

struct PalmTurret : Enemy
{
	enum Action
	{
		IDLE,
		CHARGE,
		FIRE,
		RECOVER,
		Count
	};

	Tileset *ts;

	double attentionRadius;
	double ignoreRadius;

	int firingCounter;

	int animationFactor;
	double bulletSpeed;

	sf::Vertex laserQuad[4];
	double laserAngle;
	V2d laserCenter;
	double finalLaserWidth;
	double currLaserWidth;
	double laserLength;
	void UpdateLaserWidth(double w);
	bool CheckHitPlayer(int index = 0);

	CollisionBody laserBody;

	PalmTurret(ActorParams *ap);

	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	void ProcessState();
	void SetLevel(int lev);
	void UpdateSprite();
	void ResetEnemy();
	void StartCharge();
};

#endif