#ifndef __ENEMY_BIRD_H__
#define __ENEMY_BIRD_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "PlayerComboer.h"
#include "Enemy_BirdShuriken.h"
#include "RandomPicker.h"
#include "BossStageManager.h"

struct BirdPostFightScene;
struct BirdPostFight2Scene;
struct BirdPostFight3Scene;

struct Bat;

struct Bird : Enemy
{
	enum Action
	{
		DECIDE,
		COMBOMOVE,
		PUNCH,
		KICK,
		MOVE_NODE_LINEAR,
		MOVE_NODE_QUADRATIC,
		MOVE_CHASE,
		RUSH,
		MOVE,
		SHURIKEN_SHOTGUN,
		UNDODGEABLE_SHURIKEN,
		SUMMON,
		SEQ_WAIT,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	int moveFrames;
	void NextStage();

	BossStageManager stageMgr;

	RandomPicker *decidePickers;

	RandomPicker nodePicker;
	std::vector<PoiInfo*> *nodeAVec;

	BirdPostFightScene * postFightScene;
	BirdPostFight2Scene *postFightScene2;
	BirdPostFight3Scene *postFightScene3;

	int invincibleFrames;

	CircleGroup *nodeDebugCircles;
	
	BasicAirEnemyParams *batParams;
	const static int NUM_BATS = 5;
	Bat *bats[NUM_BATS];
	int numActiveBats;
	int currMaxActiveBats;
	int numBatsToSummonAtOnce;

	std::string nodeAStr;

	BirdShurikenPool shurPool;

	PlayerComboer playerComboer;
	EnemyMover enemyMover;

	int fireCounter;

	Tileset *ts_bulletExplode;
	int comboMoveFrames;

	int reachPointOnFrame[A_Count];

	bool hitPlayer;

	BirdCommand actionQueue[3];
	int actionQueueIndex;

	int targetPlayerIndex;

	HitboxInfo hitboxInfos[A_Count];

	Tileset *ts_punch;
	Tileset *ts_kick;
	Tileset *ts_move;

	int counterTillAttack;

	V2d targetPos;
	int framesToArrive;

	Bird(ActorParams *ap);
	~Bird();
	void Setup();
	int SetLaunchersStartIndex(int ind);
	void Wait();
	void Decide(int frames);
	void ProcessHit();
	void StartFight();
	void HandleSummonedChildRemoval(Enemy *e);
	void LoadParams();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void DirectKill();
	void SetCommand(int index, BirdCommand &bc);
	void UpdatePreFrameCalculations();
	void ProcessState();
	void UpdateHitboxes();
	void DebugDraw(sf::RenderTarget *target);
	bool CanSummonBat();
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	bool CanBeHitByPlayer();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	void SetHitboxInfo(int a);


};

#endif