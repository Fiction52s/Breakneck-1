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
	double attentionRadius;
	double ignoreRadius;

	Tileset *ts;
	double maxSpeed;
	double accel;
	sf::CircleShape testCircle;
	//Shield *shield;
	Tileset *ts_bulletExplode;

	Chess(ActorParams *ap);
	~Chess();
	void StartRush();
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void FrameIncrement();
	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();

	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(int playerIndex,
		BasicBullet *b, int hitResult);
	void DirectKill();


	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif