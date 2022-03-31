#ifndef __ENEMY_GREYSKELETON_H__
#define __ENEMY_GREYSKELETON_H__

#include "Boss.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_Thorn.h"
#include "Enemy_Hand.h"
#include "Enemy_AppearingShape.h"
#include "Enemy_BeamBomb.h"

struct FinalSkeletonPostFightScene;
struct GreyEye;

struct GreySkeleton : Boss
{
	enum Action
	{
		WAIT,
		COMBOMOVE,
		MOVE,
		THORN_TEST,
		CREATE_HANDS,
		SLAP_TEST,
		HAND_TEST,
		BOMB_TEST,
		SHAPE_TEST,
		SEQ_WAIT,
		EYE_TEST,
		HIDE,
		A_Count
	};

	bool isBonus;

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;

	Tileset *ts_move;

	FinalSkeletonPostFightScene *postFightScene;

	ThornPool thornPool;

	AppearingShapePool shapePool;

	BeamBombPool bombPool;

	const static int NUM_EYES = 6;
	GreyEye *eyes[NUM_EYES];

	Hand *leftHand;
	Hand *rightHand;

	GreySkeleton(ActorParams *ap);

	~GreySkeleton();

	//Enemy functions
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	int GetNumSimulationFramesRequired();
	void Setup();

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