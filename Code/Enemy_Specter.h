#ifndef __ENEMY_SPECTER_H__
#define __ENEMY_SPECTER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Enemy_SpecterBullet.h"

struct Specter;
struct SpecterArea : QuadTreeEntrant
{
	SpecterArea(Specter *sp);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	void SetRadius(int rad);
	int radius;
	V2d position;
	sf::Rect<double> testRect;
	//CollisionBox barrier;
	Specter *specter;
	void UpdatePosition();
};

struct SpecterTester : QuadTreeCollider
{
	SpecterTester(Enemy *en);
	void HandleEntrant(QuadTreeEntrant *qte);
	void Query( sf::Rect<double> &r );
	Enemy *enemy;
};

struct Specter : Enemy
{
	enum Action
	{
		A_IDLE,
		A_ATTACK,
		A_RECOVER,
		A_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;

	SpecterBulletPool bulletPool;

	double bulletAccel;
	double bulletMaxSpeed;

	Tileset *ts_bulletExplode;

	Specter(ActorParams *ap );
	~Specter();
	void AddToGame();
	void ResetEnemy();
	void SetLevel(int lev);
	void DirectKill();
	void ProcessState();
	void UpdateSprite();
	void EnemyDraw(sf::RenderTarget *target);

	bool CanTouchSpecter();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif