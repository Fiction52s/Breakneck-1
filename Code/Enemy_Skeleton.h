#ifndef __ENEMY_SKELETON_H__
#define __ENEMY_SKELETON_H__

#include "Boss.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_SkeletonLaser.h"


struct SkeletonPostFightScene;
struct CoyoteHelper;

struct Skeleton : Boss, RayCastHandler
{
	enum Action
	{
		WAIT,
		PLAN_PATTERN,
		PATTERN_MOVE,
		MOVE_WIRE_DASH,
		DASH_RECOVER,
		MOVE_OTHER,
		COMBOMOVE,
		MOVE,
		SEQ_WAIT,
		JUMPSQUAT,
		HOP,
		LAND,
		SHOOT_LASER,
		SHOOT_LASER_HOMING,
		REDIRECT_TEST,
		GATHER_ENERGY_START,
		GATHER_ENERGY_LOOP,
		GATHER_ENERGY_END,
		LASER_SPAM,
		CHASE_MOVE,
		A_Count
	};

	enum PatternMoveType
	{
		PM_RISING_ZIP,
		PM_SWING,
		PM_HOP,
	};

	SkeletonLaserPool laserPool;
	PatternMoveType currentPatternMoveType;
	PoiInfo *currNode;
	RandomPicker patternTypePicker;

	double extraHeight;
	double eyeExtraHeight;

	RandomPicker patternOrderPicker;
	sf::CircleShape patternPreview;
	sf::Text patternNumberText;
	int patternFlickerFrames;
	int numPatternMoves;
	std::vector<PoiInfo*> pattern;
	std::vector<int> patternType;
	std::vector<int> patternOrder;
	int patternIndex;

	V2d hopTarget;
	double hopSpeed;
	double hopExtraHeight;

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;

	SkeletonPostFightScene *postFightScene;
	CoyoteHelper *coyHelper;

	//CircleGroup testGroup;
	//CircleGroup testGroup2;

	//Tileset *ts_punch;
	//Tileset *ts_kick;
	//Tileset *ts_move;

	Tileset *ts_charge;
	Tileset *ts_stand;
	Tileset *ts_hop;
	Tileset *ts_laser;

	bool ignorePointsCloserThanPlayer;
	double playerDist;

	sf::Vertex wireQuad[4];

	Skeleton(ActorParams *ap);
	~Skeleton();

	//Enemy functions
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	int GetNumSimulationFramesRequired();
	void AddToGame();

	void SetPatternLength(int len);
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
	void StartMovement(V2d &pos);


	//My functions
	void SeqWait();
	void StartFight();
	void LoadParams();
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	void Hop(V2d &pos, double p_hopSpeed, double p_hopExtraHeight);
	void FinishPatternMove();
	void UpdateWireQuad();
	V2d GetCenter();
	void FrameIncrement();
	/*void ShootStandingLaser(
		int shootType,
		V2d &dir);*/
	

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