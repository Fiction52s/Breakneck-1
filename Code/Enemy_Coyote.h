#ifndef __ENEMY_COYOTE__
#define __ENEMY_COYOTE__

#include "Bullet.h"
#include "Movement.h"
#include "Enemy_CoyoteBullet.h"
#include "SummonGroup.h"
#include "Boss.h"

struct PulseAttackPool;

struct CoyotePostFightScene;

struct Coyote : Boss, Summoner, RayCastHandler,
	LauncherEnemy//, BonusHandler
{
	enum Action
	{
		WAIT,
		SEQ_WAIT,
		MOVE,
		RUSH,
		PLAN_PATTERN,
		PATTERN_MOVE,
		PATTERN_RUSH,
		PATTERN_BULLETS,
		PATTERN_PULSE,
		DANCE_PREP,
		DANCE,
		SUMMON,
		COMBOMOVE,
		TEST_POST,
		A_Count
	};
	
	int bounceCounter;

	NodeGroup nodeGroupA;

	PoiInfo *danceNode;

	GameSession *myBonus;

	SummonGroup fireflySummonGroup;
	SummonGroup babyScorpionGroup;

	PulseAttackPool *pulsePool;
	

	V2d currBabyScorpPos;

	CoyotePostFightScene *postFightScene;

	Tileset *ts_move;
	Tileset *ts_bulletExplode;

	CoyoteBulletPool stopStartPool;

	PoiInfo *currNode;
	RandomPicker patternTypePicker;
	sf::CircleShape patternPreview;
	int patternFlickerFrames;
	int numPatternMoves;
	std::vector<PoiInfo*> pattern;
	std::vector<int> patternType;
	int patternIndex;

	Coyote(ActorParams *ap);
	~Coyote();

	//launcher functions
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult);
	//summoner functions
	void InitEnemyForSummon(SummonGroup *group, Enemy *e);

	//enemy functions
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	int SetLaunchersStartIndex(int ind);

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