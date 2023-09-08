#ifndef __ENEMY_COYOTE_HELPER__
#define __ENEMY_COYOTE_HELPER__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "EnemyMover.h"
#include "Enemy_CoyoteBullet.h"

struct GameSession;

struct Skeleton;

struct CoyoteHelper : Enemy
{
	enum Action
	{
		MOVE,
		WAIT,
		COMBOMOVE,
		SEQ_WAIT,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	Skeleton *skeleton;
	GameSession *myBonus;

	Tileset *ts_move;

	int moveFrames;
	int waitFrames;

	std::string nodeAStr;

	CoyoteBulletPool stopStartPool;

	EnemyMover enemyMover;

	int fireCounter;

	int comboMoveFrames;

	int reachPointOnFrame[A_Count];

	bool hitPlayer;

	int targetPlayerIndex;

	HitboxInfo hitboxInfos[A_Count];

	int counterTillAttack;

	V2d targetPos;
	int framesToArrive;

	CoyoteHelper(ActorParams *ap);

	void Setup();
	void Wait();
	void StartFight();
	bool CheckHitPlayer(int index);
	void LoadParams();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	//void SetCommand(int index, BirdCommand &bc);
	void ProcessState();
	void UpdateHitboxes();
	void DebugDraw(sf::RenderTarget *target);

	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();

	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	void SetHitboxInfo(int a);


};

#endif