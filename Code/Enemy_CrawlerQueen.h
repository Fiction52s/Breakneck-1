#ifndef __ENEMY_CRAWLERQUEEN_H__
#define __ENEMY_CRAWLERQUEEN_H__

#include "Enemy.h"
#include "ObjectPool.h"
#include <sfeMovie/Movie.hpp>
#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "PlayerComboer.h"
#include "Enemy_TigerGrindBullet.h"
#include "RandomPicker.h"
#include "SummonGroup.h"

struct Crawler;

struct Actor;
struct EffectPool;
struct StorySequence;

struct CrawlerPostFightScene;
struct CrawlerPostFight2Scene;
struct QueenFloatingBomb;



struct PoiInfo;
struct CrawlerQueen : Enemy, SurfaceMoverHandler,
	EnemyMoverHandler, Summoner
{
	enum Action
	{
		DECIDE,
		MOVE,
		COMBOMOVE,
		SUMMON,
		DIG_IN,
		UNDERGROUND,
		LUNGE,
		DIG_OUT,
		SLASH,
		SEQ_WAIT,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	std::vector<PoiInfo*> *nodeVecA;

	RandomPicker decidePicker;
	RandomPicker digDecidePicker;
	RandomPicker nodePicker;
	RandomPicker undergroundNodePicker;

	double currDashSpeed;
	double currDashAccel;

	double lungeSpeed;

	int spriteGoalAngle;
	bool wasAerial;

	CrawlerPostFightScene *postFightScene;

	CrawlerPostFight2Scene *postFightScene2;

	PoiInfo *targetNode;

	Tileset *ts_move;
	Tileset *ts_slash;
	Tileset *ts_dig_in;
	Tileset *ts_dig_out;
	Tileset *ts_jump;

	int moveFrames;

	std::string nodeAStr;

	PlayerComboer playerComboer;
	EnemyMover enemyMover;

	SummonGroup crawlerSummonGroup;

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

	int invincibleFrames;

	const static int NUM_BOMBS = 5;
	QueenFloatingBomb *bombs[NUM_BOMBS];
	int currMaxActiveBombs;

	void StartAngryYelling();
	void StartInitialUnburrow();




	CrawlerQueen(ActorParams *ap);
	~CrawlerQueen();
	void ProcessHit();
	void Decide(int numFrames);
	void GoUnderground(int numFrames);
	void Setup();
	void Wait();
	void StartFight();
	void LoadParams();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void DirectKill();
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
	bool CanBeHitByPlayer();
	bool IsDecisionValid(int d);

	void SetHitboxInfo(int a);
	void HitTerrainAerial(Edge *, double);


	void HandleFinishTargetedMovement();
	void InitEnemyForSummon(
		SummonGroup *group,
		Enemy *e);


};

#endif