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
	sf::Rect<double> testRect;
	CollisionBox barrier;
	Specter *specter;
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

	void UpdateSprite();
	void UpdateHitboxes();

	void ResetEnemy();

	SpecterArea *myArea;

	int radius;

	sf::Sprite sprite;
	Tileset *ts;

	sf::CircleShape radCircle;

	int animationFactor;
};

#endif