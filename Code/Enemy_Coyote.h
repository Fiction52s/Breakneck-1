#ifndef __ENEMY_COYOTE__
#define __ENEMY_COYOTE__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "PlayerComboer.h"
#include "Enemy_CoyoteBullet.h"
#include "BossStageManager.h"
#include "RandomPicker.h"
#include "SummonGroup.h"

struct CoyotePostFightScene;



struct Coyote : Enemy, Summoner
{
	enum Action
	{
		SEQ_WAIT,
		MOVE,
		WAIT,
		SUMMON,
		COMBOMOVE,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	std::vector<PoiInfo*> *nodeAVec;

	RandomPicker nodePicker;
	SummonGroup fireflySummonGroup;

	CoyotePostFightScene *postFightScene;

	int invincibleFrames;

	BossStageManager stageMgr;
	RandomPicker *decidePickers;

	Tileset *ts_move;



	int moveFrames;
	int waitFrames;

	std::string nodeAStr;

	CoyoteBulletPool stopStartPool;

	PlayerComboer playerComboer;
	EnemyMover enemyMover;

	void NextStage();
	bool stageChanged;

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
	bool IsDecisionValid(int d);
	void ChooseNextAction();
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

	void InitEnemyForSummon(SummonGroup *group, Enemy *e);

	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	void SetHitboxInfo(int a);


};

#endif