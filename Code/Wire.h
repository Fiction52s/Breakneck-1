#ifndef __WIRE_H__
#define __WIRE_H__

#include <SFML/Graphics.hpp>
#include "Physics.h"
#include "QuadTree.h"
#include "Tileset.h"
#include <SFML/System/Clock.hpp>
#include "EnemyTracker.h"
#include "VectorMath.h"

struct WirePoint
{
	V2d pos;
	V2d edgeEnd;
	V2d test;
	double angleDiff;
	double quantity;
	double sortingAngleDist;
	Edge *e;
	Enemy *enemy;
	int enemyPosIndex;
	bool start;
	bool clockwise;

	WirePoint()
	{
		e = NULL;
		enemy = NULL;
	}
};


struct Wire;
struct WireCharge
{
	enum Action
	{
		INACTIVE,
		RETRACTING,
		//EXPLODING
	};

	WireCharge *next;
	WireCharge *prev;
	Action action;
	Wire *wire;
	V2d position;
	int edgeIndex;
	double edgeQuantity;
	CollisionBox hitbox;
	int vaIndex;

	WireCharge(Wire *w, int vIndex);
	void Reset();
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void UpdateSprite();
	void ClearSprite();
	void HitEnemy();
};

struct Actor;
struct Wire : RayCastHandler, QuadTreeCollider,
	EnemyTracker
{
	enum WireState
	{
		IDLE,
		FIRING,
		HIT,
		PULLING,
		RETRACTING,
		RELEASED,
		HITENEMY,
	};

	const static int MAX_POINTS = 64;
	const static int MAX_CHARGES = 16;

	bool foundPoint;
	WirePoint anchor;
	WirePoint points[MAX_POINTS];
	WireState state;


	V2d hitEnemyDelta;
	V2d closestPoint;
	V2d realAnchor;
	V2d oldPos;
	V2d storedPlayerPos;
	V2d retractPlayerPos;
	V2d currOffset;
	V2d fireDir;
	V2d anchorVel;
	V2d quadOldPosA;
	V2d quadOldWirePosB;
	V2d quadWirePosC;
	V2d quadPlayerPosD;

	double closestDiff;
	double retractSpeed;
	double fuseQuantity;
	bool canRetractGround;
	double maxTotalLength;
	double maxFireLength;
	double quadHalfWidth;
	double fireRate;
	double minSideOther;
	double minSideAlong;
	double totalLength;
	double segmentLength;
	double minSegmentLength;
	double pullStrength;
	double pullAccel;
	double maxPullStrength;
	double startPullStrength;
	double dragStrength;
	double dragAccel;
	double maxDragStrength;
	double startDragStrength;
	double grassCheckRadius;
	double rcCancelDist;
	double rcQuant;

	int hitEnemyFrame;
	int hitEnemyFramesTotal;
	int extraBuffer; //when swinging around edges sometimes the wire
					 //stretches some. This is attemping to hole up that problem. if it happens
					 //too much then I can go into it and solve the real problems.
	int numAnimFrames;
	int firingTakingUp;
	int addedPoints;
	int numVisibleIndexes;
	int fusePointIndex;
	int newWirePoints;
	int numTotalCharges;
	int frame;
	int animFactor;
	int numPoints;
	int aimingPrimaryAngleRange;
	int numQuadVertices;
	int numMinimapQuads;
	int framesFiring;
	int triggerThresh;
	int hitStallFrames;
	int hitStallCounter;
	int antiWireGrassCount;
	
	CollisionBox movingHitbox;
	sf::Vector2i offset;
	
	bool right;
	bool clockwise;
	bool triggerDown;
	bool prevTriggerDown;

	WireCharge *activeChargeList;
	WireCharge *inactiveChargeList;

	sf::Sprite wireTip;
	sf::Sprite fuseSprite;
	
	Tileset *ts_wireTip;
	Tileset *ts_wire;
	Tileset *ts_miniHit;
	Tileset *ts_wireCharge;
	
	sf::VertexArray chargeVA;

	CollisionBox testHitbox;
	HitboxInfo *tipHitboxInfo;
	sf::Vertex *quads;
	sf::Vertex *minimapQuads;
	
	Edge *minSideEdge;
	Edge *rcEdge;
	Actor *player;

	sf::Rect<double> grassQueryBox;
	std::string queryMode;
	std::list<sf::Drawable*> progressDraw;

	Wire( Actor *player, bool right );
	~Wire();
	void UpdateAnchors( V2d vel );
	void UpdateAnchors2( V2d vel );
	void SetFireDirection( V2d dir );
	void UpdateEnemyAnchor();
	bool TryFire();
	void Check();
	void UpdateMinimapQuads( sf::View &uiView );
	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
	void UpdateState( bool touchEdgeWithWire );
	void Draw( sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw( sf::RenderTarget *target );
	void ClearDebug();
	void HandleEntrant( QuadTreeEntrant *qte );
	void TestPoint( Edge * e);
	void SwapPoints( int aIndex, int bIndex );
	void UpdateQuads();
	void Reset();
	bool IsValidTrackEnemy(Enemy *e);
	sf::Vector2<double> GetOriginPos( bool test );
	void UpdateFuse();
	double GetSegmentLength();
	sf::Vector2<double> GetPlayerPos();
	double GetCurrentTotalLength();
	void HitEnemy(V2d &pos);
	void DrawWireCharges( sf::RenderTarget *target );
	void CreateWireCharge();
	void DeactivateWireCharge( WireCharge *wc );
	WireCharge * GetWireCharge();
	void ActivateWireCharge( int index );
	void ClearCharges();
	void DestroyDeactivatedCharges();
	void ActivateRetractionCharges();
	void UpdateChargesPhysics();
	void UpdateChargesSprites();
	void UpdateChargesPrePhysics();
	void UpdateChargesPostPhysics();
	void CheckAntiWireGrass();
	void TestPoint2( Edge *e );
	double GetTestPointAngle( Edge *e );
	int CountActiveCharges();
	int CountInactiveCharges();
	void ActivateCharges();
	void Retract();
	CollisionBox *GetTipHitbox();
	void SortNewPoints();


};

#endif