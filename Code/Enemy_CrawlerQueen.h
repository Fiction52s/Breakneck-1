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

struct Crawler;

struct Actor;
struct EffectPool;
struct StorySequence;

struct CrawlerPostFightScene;
struct CrawlerPostFight2Scene;
struct QueenFloatingBomb;

struct PoiInfo;
struct CrawlerQueen : Enemy, SurfaceMoverHandler,
	EnemyMoverHandler
{
	enum Action
	{
		DECIDE,
		MOVE,
		COMBOMOVE,
		SUMMON,
		DIG,
		SLASH,
		SEQ_WAIT,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	double currDashSpeed;
	double currDashAccel;

	int spriteGoalAngle;
	bool wasAerial;

	CrawlerPostFightScene *postFightScene;

	CrawlerPostFight2Scene *postFightScene2;

	PoiInfo *targetNode;

	Tileset *ts_move;
	Tileset *ts_slash;

	int moveFrames;
	int waitFrames;

	int numActiveCrawlers;

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

	ActorParams *crawlerParams;

	const static int NUM_CRAWLERS = 5;
	Crawler *crawlers[NUM_CRAWLERS];
	int currMaxActiveCrawlers;
	int numCrawlersToSummonAtOnce;


	const static int NUM_BOMBS = 5;
	QueenFloatingBomb *bombs[NUM_BOMBS];
	int currMaxActiveBombs;

	void StartAngryYelling();
	void StartInitialUnburrow();




	CrawlerQueen(ActorParams *ap);
	~CrawlerQueen();
	void ProcessHit();
	void Decide(int numFrames);
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
	void HandleCrawlerDeath();
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();

	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	bool CanSummonCrawler();

	void SetHitboxInfo(int a);


	void HandleFinishTargetedMovement();


};

#endif