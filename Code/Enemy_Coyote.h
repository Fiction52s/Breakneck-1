#ifndef __ENEMY_COYOTE__
#define __ENEMY_COYOTE__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "PlayerComboer.h"
#include "Enemy_CoyoteBullet.h"

struct CoyotePostFightScene;

struct Coyote : Enemy
{
	enum Action
	{
		SEQ_WAIT,
		MOVE,
		WAIT,
		COMBOMOVE,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};


	CoyotePostFightScene *postFightScene;

	Tileset *ts_move;



	int moveFrames;
	int waitFrames;

	std::string nodeAStr;

	CoyoteBulletPool stopStartPool;

	PlayerComboer playerComboer;
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

	Coyote(ActorParams *ap);
	~Coyote();
	void LoadParams();
	void ProcessHit();
	int GetNumStoredBytes();
	void Setup();
	void StartFight();
	void Wait();
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