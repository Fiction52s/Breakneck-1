#ifndef __ENEMY_BIRD_H__
#define __ENEMY_BIRD_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "PlayerComboer.h"

struct Bird : Enemy
{
	enum Action
	{
		MOVE,
		WAIT,
		COMBOMOVE,
		PUNCH,
		KICK,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	int moveFrames;
	int waitFrames;

	PlayerComboer playerComboer;
	EnemyMover enemyMover;

	int fireCounter;

	Tileset *ts_bulletExplode;
	Tileset *ts_aura;
	int comboMoveFrames;

	int reachPointOnFrame[A_Count];

	bool hitPlayer;

	BirdCommand actionQueue[3];
	int actionQueueIndex;

	int targetPlayerIndex;
	LineMovement *move;
	MovementSequence ms;

	HitboxInfo hitboxInfos[A_Count];

	Tileset *ts_punch;
	Tileset *ts_kick;
	Tileset *ts_move;

	int counterTillAttack;

	V2d targetPos;
	int framesToArrive;

	Bird(ActorParams *ap);

	void LoadParams();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void DirectKill();
	void SetCommand(int index, BirdCommand &bc);
	void UpdatePreFrameCalculations();
	void ProcessState();
	void UpdateHitboxes();

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