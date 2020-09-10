#ifndef __ENEMY_TIGER_H__
#define __ENEMY_TIGER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "PlayerComboer.h"
#include "Enemy_TigerGrindBullet.h"

struct PoiInfo;
struct Tiger : Enemy
{
	enum Action
	{
		MOVE,
		WAIT,
		COMBOMOVE,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	PoiInfo *targetNode;

	Tileset *ts_move;

	int moveFrames;
	int waitFrames;

	std::string nodeAStr;

	TigerGrindBulletPool snakePool;

	PlayerComboer playerComboer;
	EnemyMover enemyMover;

	int fireCounter;

	Tileset *ts_bulletExplode;
	Tileset *ts_aura;
	int comboMoveFrames;

	int reachPointOnFrame[A_Count];

	bool hitPlayer;

	int targetPlayerIndex;

	HitboxInfo hitboxInfos[A_Count];

	int counterTillAttack;

	V2d targetPos;
	int framesToArrive;

	Tiger(ActorParams *ap);

	void LoadParams();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void DirectKill();
	//void SetCommand(int index, BirdCommand &bc);
	void UpdatePreFrameCalculations();
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