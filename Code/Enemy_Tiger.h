#ifndef __ENEMY_TIGER_H__
#define __ENEMY_TIGER_H__

#include "Boss.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_TigerGrindBullet.h"

struct TigerPostFightScene;
struct TigerPostFight2Scene;

struct Tiger : Boss
{
	enum Action
	{
		WAIT,
		MOVE_GRIND,
		MOVE_JUMP,
		COMBOMOVE,
		SEQ_WAIT,
		A_Count
	};

	

	TigerPostFightScene *postFightScene;
	TigerPostFight2Scene *postFightScene2;

	Tileset *ts_move;

	TigerGrindBulletPool snakePool;

	NodeGroup nodeGroupA;

	Tiger(ActorParams *ap);
	~Tiger();

	//Enemy functions
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();

	//Boss functions
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

	//My functions
	void SeqWait();
	void StartFight();
	void LoadParams();

	//Rollback
	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

};

#endif