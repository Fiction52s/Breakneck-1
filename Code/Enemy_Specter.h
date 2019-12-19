#ifndef __ENEMY_SPECTER_H__
#define __ENEMY_SPECTER_H__

#include "Enemy.h"

struct Specter;
struct SpecterArea : QuadTreeEntrant
{
	SpecterArea(Specter *sp, sf::Vector2i &pos, int rad);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	int radius;
	V2d position;
	sf::Rect<double> testRect;
	//CollisionBox barrier;
	Specter *specter;
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
	//MovementSequence testSeq;
	Specter(GameSession *owner,
		bool hasMonitor,
		sf::Vector2i & pos,
		int p_level );
	~Specter();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);

	bool CanTouchSpecter();
	

	void UpdateSprite();

	void ResetEnemy();

	SpecterArea *myArea;

	int radius;

	sf::Sprite sprite;
	Tileset *ts;

	sf::CircleShape radCircle;

	int animationFactor;
};

#endif