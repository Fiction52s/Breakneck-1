#ifndef __ENEMY_CHESS_H__
#define __ENEMY_CHESS_H__

#include "Enemy.h"
#include "Bullet.h"

struct Chess : Enemy
{
	enum Action
	{
		NEUTRAL,
		RUSH,
		RECOVER,
		CHASE,
		A_Count
	};

	enum ChessType
	{
		HORIZ,
		VERT,
		DIAGDOWNRIGHT,
		DIAGUPRIGHT,
	};

	int chessType;
	double attentionRadius;
	double ignoreRadius;

	Tileset *ts;
	V2d velocity;
	double maxSpeed;
	double accel;
	sf::CircleShape testCircle;
	Shield *shield;


	Chess(ActorParams *ap);
	~Chess();
	void StartRush();
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();
};

#endif