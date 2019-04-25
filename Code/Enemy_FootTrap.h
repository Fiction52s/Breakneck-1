#ifndef __ENEMY_FOOTTRAP_H__
#define __ENEMY_FOOTTRAP_H__

#include "Enemy.h"

struct FootTrap : Enemy
{
	enum Action
	{
		LATENT,
		CHOMPING,
	};

	Action action;
	int actionLength[CHOMPING + 1];

	FootTrap(GameSession *owner, bool hasMonitor,
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

	double angle;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	int animationFactor;
	sf::Vector2<double> gn;
};

#endif