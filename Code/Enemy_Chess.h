#ifndef __ENEMY_CHESS_H__
#define __ENEMY_CHESS_H__

#include "Enemy.h"
#include "Bullet.h"

struct Chess : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		RUSH,
		RECOVER,
		RETURN,
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

	struct MyData : StoredEnemyData
	{
		V2d velocity;
		int fireCounter;
	};
	MyData data;

	int chessType;

	Tileset *ts;
	double maxSpeed;
	double accel;
	sf::CircleShape testCircle;

	Chess(ActorParams *ap);
	~Chess();
	void StartRush();
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();

	void FrameIncrement();
	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif