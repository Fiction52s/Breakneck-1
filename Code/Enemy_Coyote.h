#ifndef __ENEMY_COYOTE__
#define __ENEMY_COYOTE__

#include "Bullet.h"
#include "Movement.h"
#include "Enemy_CoyoteBullet.h"
#include "SummonGroup.h"
#include "Boss.h"

struct CoyotePostFightScene;



struct Coyote : Boss, Summoner
{
	enum Action
	{
		WAIT,
		SEQ_WAIT,
		MOVE,
		SUMMON,
		COMBOMOVE,
		A_Count
	};
	
	NodeGroup nodeGroupA;

	SummonGroup fireflySummonGroup;

	CoyotePostFightScene *postFightScene;

	Tileset *ts_move;

	CoyoteBulletPool stopStartPool;

	Coyote(ActorParams *ap);
	~Coyote();

	//summoner functions
	void InitEnemyForSummon(SummonGroup *group, Enemy *e);

	//enemy functions
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();

	//boss functions
	bool TryComboMove(V2d &comboPos, int comboMoveDuration,
		int moveDurationBeforeStartNextAction,
		V2d &comboOffset);
	int ChooseActionAfterStageChange();
	void ActivatePostFightScene();
	void ActionEnded();
	void HandleAction();
	void StartAction();
	void SetupPostFightScenes();
	void SetupNodeVectors();
	bool IsDecisionValid(int d);
	bool IsEnemyMoverAction(int a);

	//my functions
	void LoadParams();
	void StartFight();
	void SeqWait();
	

	//rollback functions
	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif