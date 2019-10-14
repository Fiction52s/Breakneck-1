#ifndef __ENEMY_BOUNCEFLOATER_H__
#define __ENEMY_BOUNCEFLOATER_H__

#include "Enemy.h"

struct ComboObject;

struct BounceFloater : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_BOUNCE,
		S_RECOVER,
		S_Count
	};

	BounceFloater(GameSession *owner,
		sf::Vector2i pos, int p_level);
	//void HandleEntrant(QuadTreeEntrant *qte);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void HandleNoHealth();

	V2d origPos;

	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];

	sf::Sprite sprite;
	Tileset *ts;
};

#endif