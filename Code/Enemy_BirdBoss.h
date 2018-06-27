#ifndef __ENEMY_BIRDBOSS_H__
#define __ENEMY_BIRDBOSS_H__

#include "Enemy.h"
#include "ObjectPool.h"

struct GravRing : Enemy, SurfaceMoverHandler, PoolMember
{
	enum Action
	{
		FLOATING,
		EXPLODING,
		Count
	};

	Action action;
	GravRing(GameSession *owner, ObjectPool *myPool, int index);
	int actionLength[Count];
	int animFactor[Count];
	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	SurfaceMover *mover;
	Tileset *ts;
	HitboxInfo *hitboxInfo;
	sf::Sprite sprite;
	ObjectPool *myPool;
	void Init(V2d pos, V2d vel);
	void ProcessState();
	void HandleNoHealth();
	void HitTerrainAerial(Edge *, double);
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void ProcessHit();
};

struct BirdBoss : Enemy
{
	enum RingType
	{
		NORMAL_SMALL,
		NORMAL_LARGE
	};

	enum ChoiceType
	{
		C_FLYTOWARDS,
		C_FOCUS,
		C_PUNCH,
		C_RINGTHROW,
		C_GRAVITYCHOOSE,
		C_SUPERKICK,
		C_Count
	};

	struct ChoiceParams
	{
		ChoiceType cType;
		int focusRadius;
		RingType rType;
		float moveSpeed;
	};

	enum Action
	{
		WAIT,
		MOVE,
		STARTGLIDE,
		GLIDE,
		ENDGLIDE,
		STARTFOCUS,
		FOCUSLOOP,
		ENDFOCUS,
		FOCUSATTACK,
		STARTPUNCH,
		HOLDPUNCH,
		PUNCH,
		RINGTHROW,
		GRAVITYCHOOSE,
		AIMSUPERKICK,
		SUPERKICK,
		S_Count
	};

	BirdBoss( GameSession *owner, sf::Vector2i &pos );

	void BeginMove();
	void BeginFocus();
	void BeginPunch();
	void BeginThrow();
	void BeginSuperKick();
	void BeginGravityChoose();
	~BirdBoss();
	/*void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);*/
	void ActionEnded();
	void ProcessState();
	void HandleHitAndSurvive();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void HandleNoHealth();
	void PlanChoice( int ind );
	void BeginChoice();
	void NextChoice();

	ChoiceParams *choices;
	int numChoices;

	int actionLength[S_Count];
	int animFactor[S_Count];
	Tileset *tilesets[S_Count];
	Action action;

	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	sf::Sprite sprite;

	HitboxInfo *bodyHitboxInfo;
	HitboxInfo *focusHitboxInfo;
	HitboxInfo *punchHitboxInfo;
	HitboxInfo *superkickHitboxInfo;

	V2d startMovePos;
	V2d endMovePos;
	V2d origPos;
	double moveSpeed;
	double moveAccel;

	int maxTrackingFrames;
	int currTrackingFrame;
	V2d trackingPos;

	V2d punchVel;
	V2d maxPunchVel;


	int superKickWaitChoices;

	//HitboxInfo *punchHitboxInfo;
	//HitboxInfo *ring0HitboxInfo;
	//HitboxInfo *ring1HitboxInfo;
	float focusRadius;

	bool facingRight;
};

#endif