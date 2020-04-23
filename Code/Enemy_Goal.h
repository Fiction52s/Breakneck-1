#ifndef __ENEMY_GOAL_H__
#define __ENEMY_GOAL_H__

#include "Enemy.h"

struct ActorParams;

struct Goal : Enemy
{
	enum Action
	{
		A_SITTING,
		A_KINKILLING,
		A_EXPLODING,
		A_DESTROYED,
		A_Count
	};


	Goal(ActorParams *ap);
	~Goal();
	//static Enemy *Create(std::ifstream &is);
	void DrawMinimap(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void ProcessState();
	void ConfirmKill();
	int explosionLength;

	int explosionAnimFactor;
	int explosionYOffset;
	int initialYOffset;

	sf::Sprite sprite;
	sf::Sprite miniSprite;
	Tileset *ts;
	Tileset *ts_mini;
	Tileset *ts_explosion;
	Tileset *ts_explosion1;
	float goalKillStartZoom;
	sf::Vector2f goalKillStartPos;

	int world;

	//int frame;
	int deathFrame;
	int animationFactor;
	bool dead;
	sf::Vector2<double> gn;
};


#endif