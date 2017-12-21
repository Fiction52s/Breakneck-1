#ifndef __ENEMY_SPRING_H__
#define __ENEMY_SPRING_H__

#include "Enemy.h"

struct Spring : Enemy
{
	//MovementSequence testSeq;
	Spring(GameSession *owner,
		sf::Vector2i &pos, sf::Vector2i &other, int moveFrames);
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	//void HandleQuery(QuadTreeCollider * qtc);
	//bool IsTouchingBox(const sf::Rect<double> &r);

	int frame;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	//HitboxInfo *hitboxInfo;

	int animationFactor;

	void SaveEnemyState() {}
	void LoadEnemyState() {}

	sf::Vector2<double> dir;
	int speed;
	int stunFrames;
};

#endif