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
		MOVE,
		WAIT,
		COMBOMOVE,
		SEQ_WAIT,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	int spriteGoalAngle;
	bool wasAerial;

	CrawlerPostFightScene *postFightScene;

	CrawlerPostFight2Scene *postFightScene2;

	PoiInfo *targetNode;

	Tileset *ts_move;

	int moveFrames;
	int waitFrames;

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

	//double moveSpeed;

	void StartAngryYelling();
	void StartInitialUnburrow();


	CrawlerQueen(ActorParams *ap);
	~CrawlerQueen();
	void ProcessHit();
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

	void SetHitboxInfo(int a);


	void HandleFinishTargetedMovement();


};

#endif