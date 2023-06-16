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

	struct MyData : StoredEnemyData
	{
		double currLaserWidth;
	};
	MyData data;

	Tileset *ts;

	sf::Vertex laserQuad[4];
	double laserAngle;
	V2d laserCenter;
	double finalLaserWidth;
	
	double laserLength;
	
	CollisionBody laserBody;
	HitboxInfo *laserHitboxInfo;

	PalmTurret(ActorParams *ap);
	~PalmTurret();

	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	void ProcessState();
	void SetLevel(int lev);
	void UpdateSprite();
	void ResetEnemy();
	void StartCharge();
	void UpdateLaserWidth(double w);
	bool CheckHitPlayer(int index = 0);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif