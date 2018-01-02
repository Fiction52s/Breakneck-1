#ifndef __ENEMY_GOAL_H__
#define __ENEMY_GOAL_H__

#include "Enemy.h"

struct Goal : Enemy
{
	enum Action
	{
		A_SITTING,
		A_KINKILLING,
		A_EXPLODING,
		A_DESTROYED
	};

	Goal(GameSession *owner, Edge *ground, double quantity);
	void DrawMinimap(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void ProcessState();
	void ConfirmKill();

	CollisionBody *hurtBody;

	Action action;
	sf::Sprite sprite;
	sf::Sprite miniSprite;
	Tileset *ts;
	Tileset *ts_mini;
	Tileset *ts_explosion;
	float goalKillStartZoom;
	sf::Vector2f goalKillStartPos;

	Edge *ground;
	double edgeQuantity;

	

	double angle;

	//int frame;
	int deathFrame;
	int animationFactor;
	bool dead;
	sf::Vector2<double> gn;
};


#endif