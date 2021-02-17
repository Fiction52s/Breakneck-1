#ifndef __ENEMY_TIGER_H__
#define __ENEMY_TIGER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "Enemy_TigerGrindBullet.h"

struct PoiInfo;
struct TigerPostFightScene;
struct TigerPostFight2Scene;
struct Tiger : Enemy
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

	TigerPostFightScene *postFightScene;
	TigerPostFight2Scene *postFightScene2;

	PoiInfo *targetNode;

	Tileset *ts_move;

	int moveFrames;
	int waitFrames;

	std::string nodeAStr;

	TigerGrindBulletPool snakePool;

	EnemyMover enemyMover;

	int fireCounter;

	Tileset *ts_bulletExplode;
	int comboMoveFrames;

	int reachPointOnFrame[A_Count];

	bool hitPlayer;

	int targetPlayerIndex;

	HitboxInfo hitboxInfos[A_Count];

	int counterTillAttack;

	V2d targetPos;
	int framesToArrive;

	Tiger(ActorParams *ap);
	~Tiger();
	void ProcessHit();
	void Setup();
	void Wait();
	void StartFight();
	void LoadParams();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void DirectKill();
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