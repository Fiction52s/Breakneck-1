#ifndef __ENEMY_BOOSTER_H__
#define __ENEMY_BOOSTER_H__

#include "Enemy.h"

struct Booster : Enemy
{
	enum Action
	{
		NEUTRAL,
		BOOST,
		REFRESH,
		Count
	};

	Action action;
	Booster(GameSession *owner,
		sf::Vector2i &pos, int strength);
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
	int frame;

	sf::Sprite sprite;
	Tileset *ts;
	//CollisionBox hurtBody;
	CollisionBox hitBody;

	int strength;
	//HitboxInfo *hitboxInfo;

	int animationFactor;

	void SaveEnemyState() {}
	void LoadEnemyState() {}
};

#endif