#ifndef __ENEMY_HEALTHFLY_H__
#define __ENEMY_HEALTHFLY_H__

#include "Enemy.h"

struct HealthFly : Enemy
{
	enum FlyType
	{
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		WHITE,
		Count
	};

	HealthFly(GameSession *owner,
		sf::Vector2i &pos,
		FlyType fType);
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	void UpdateHitboxes();
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	bool PlayerSlowingMe();
	void DebugDraw(sf::RenderTarget *target);
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	CollisionBox hurtBody;
	CollisionBox hitBody;
	Enemy *host;
	FlyType flyType;
	sf::Sprite sprite;
	Tileset *ts;
	int frame;
	int animationFactor;
	bool caught;
};

#endif