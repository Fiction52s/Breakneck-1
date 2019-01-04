#ifndef __ENEMY_SPRING_H__
#define __ENEMY_SPRING_H__

#include "Enemy.h"

struct Spring : Enemy
{
	//MovementSequence testSeq;
	Spring(GameSession *owner,
		sf::Vector2i &pos, sf::Vector2i &other, int moveFrames);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	
	void UpdateSprite();
	
	void ResetEnemy();

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBody * hurtBody;
	CollisionBody* hitBody;
	void DebugDraw(sf::RenderTarget *target);
	//HitboxInfo *hitboxInfo;

	int animationFactor;

	sf::Vector2<double> dir;
	int speed;
	int stunFrames;

	sf::Vertex debugLine[2];
	sf::Text debugSpeed;
};

#endif