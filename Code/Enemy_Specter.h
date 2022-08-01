#ifndef __ENEMY_SPECTER_H__
#define __ENEMY_SPECTER_H__

#include "Enemy.h"

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
		IDLE,
		A_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	SpecterArea myArea;

	int radius;
	Tileset *ts;

	sf::CircleShape radCircle;

	int animationFactor;

	Specter(ActorParams *ap );
	~Specter();
	void SetLevel(int lev);
	void AddToWorldTrees();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);

	bool CanTouchSpecter();
	

	void UpdateSprite();

	void ResetEnemy();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif