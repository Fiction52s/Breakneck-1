#ifndef __ENEMY_TRAPROOT_H__
#define __ENEMY_TRAPROOT_H__

#include "Enemy.h"

struct TrapRoot : Enemy
{
	enum Action
	{
		LATENT,
		EMERGE,
		IDLE,
		ENRAGED
	};

	Action action;
	int actionLength[ENRAGED+ 1];

	TrapRoot(GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void ProcessState();

	sf::Sprite sprite;
	Tileset *ts;

	Edge *ground;
	double edgeQuantity;

	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	HitboxInfo *hitboxInfo;

	double angle;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	int animationFactor;
	sf::Vector2<double> gn;
};

#endif