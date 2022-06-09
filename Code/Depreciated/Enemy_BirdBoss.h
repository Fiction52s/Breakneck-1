#ifndef __ENEMY_BIRDBOSS_H__
#define __ENEMY_BIRDBOSS_H__

#include "Enemy.h"
#include "ObjectPool.h"

struct BirdBoss;
struct GravRing : Enemy, PoolMember
{
	enum Action
	{
		HOMING,
		ORBITING,
		EXPLODING,
		FALLING,
		Count
	};

	Action action;
	GravRing(GameSession *owner, BirdBoss *parent, ObjectPool *myPool, int index);
	int actionLength[Count];
	int animFactor[Count];
	Tileset *ts;
	sf::Sprite sprite;
	V2d fallDir;
	double grav;
	ObjectPool *myPool;
	void Init(V2d pos, V2d vel);
	V2d velocity;
	V2d orbitAxis;
	BirdBoss *parent;
	double orbitRadius;
	double maxOrbitSpeed;
	double orbitSpeed;
	void ProcessState();
	void HandleNoHealth();
	void SetFall( V2d &dir, double grav );
	//void HitTerrainAerial(Edge *, double);
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void ProcessHit();
};

struct SpaceMover;
struct BirdBoss : Enemy, RayCastHandler
{
	SpaceMover *spaceMover;
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
		SUPERKICKIMPACT,
		SUPERKICKRECOVER,

		COUNTERPUNCHWAIT,
		COUNTERPUNCHFLY,
		COUNTERPUNCHHIT,
		S_Count
	};

	void DebugDraw(sf::RenderTarget *target);
	V2d rayStart;
	V2d rayEnd;
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	Edge *rcEdge;
	double rcQuantity;
	BirdBoss( GameSession *owner, sf::Vector2i &pos );
	ObjectPool *ringPool;
	V2d superKickPoint;
	V2d superKickStart;
	V2d velocity;

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