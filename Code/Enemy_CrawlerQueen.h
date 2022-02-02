#ifndef __ENEMY_CRAWLERQUEEN_H__
#define __ENEMY_CRAWLERQUEEN_H__
#include "ObjectPool.h"
#include <sfeMovie/Movie.hpp>
#include "Bullet.h"
#include "Movement.h"
#include "EnemyMover.h"
#include "SummonGroup.h"
#include "Boss.h"

struct Crawler;

struct Actor;
struct EffectPool;
struct StorySequence;

struct CrawlerPostFightScene;
struct CrawlerPostFight2Scene;
struct QueenFloatingBomb;



struct PoiInfo;
struct CrawlerQueen : Boss, SurfaceMoverHandler, 
	Summoner
{
	enum Action
	{
		WAIT,
		MOVE,
		CHASE,
		COMBOMOVE,
		SUMMON,
		DIG_IN,
		BOOST,
		BOOSTCHARGE,
		UNDERGROUND,
		LUNGE,
		LUNGESTART,
		LUNGELAND,
		DIG_OUT,
		SLASH,
		SEQ_WAIT,
		TEST_POST,
		A_Count
	};

	RandomPicker digDecidePicker;
	RandomPicker clockwisePicker;

	double currDashSpeed;
	double currDashAccel;

	double lungeSpeed;

	int spriteGoalAngle;
	bool wasAerial;
	bool chaseOver;

	CrawlerPostFightScene *postFightScene;
	CrawlerPostFight2Scene *postFightScene2;

	PoiInfo *targetNode;

	V2d bombThrowDir;
	double bombThrowSpeed;

	GameSession *myBonus;
	

	Tileset *ts_lunge;
	Tileset *ts_move;
	Tileset *ts_slash;
	Tileset *ts_dig_in;
	Tileset *ts_dig_out;
	Tileset *ts_jump;
	Tileset *ts_bulletExplode;
	Tileset *ts_boostCharge;

	SummonGroup crawlerSummonGroup;
	SummonGroup bombSummonGroup;

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;

	CrawlerQueen(ActorParams *ap);
	~CrawlerQueen();

	//Summoner functions
	void InitEnemyForSummon(
		SummonGroup *group,
		Enemy *e);

	//Enemy functions
	void EnemyDraw(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	//void Setup();

	//Boss functions
	void RespondToTakingFullHit();
	void SetupPostFightScenes();
	void SetupNodeVectors();
	bool IsDecisionValid(int d);
	void ActionEnded();
	void HandleAction();
	void StartAction();
	int ChooseActionAfterStageChange();
	void ActivatePostFightScene();
	int GetNumSimulationFramesRequired();

	//My functions
	double GetCurrThrowSpeed();
	double GetCurrDashSpeed();
	double GetCurrBoostSpeed();
	bool GetPlayerClockwise();
	void GoUnderground(int numFrames);
	void SeqWait();
	void StartFight();
	void LoadParams();
	void StartAngryYelling();
	void StartInitialUnburrow();

	//SurfaceMover functions
	void HitTerrainAerial(Edge *, double);


	void HandleFinishTargetedMovement();
	

	//Rollback
	struct MyData : StoredEnemyData
	{
		int empty;
	};
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

};

#endif