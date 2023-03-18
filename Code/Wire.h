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
	V2d test;
	double angleDiff;
	double quantity;
	double sortingAngleDist;
	EdgeInfo edgeInfo;
	int enemyIndex;
	int enemyPosIndex;
	bool start;
	bool clockwise;

	WirePoint();
	void Reset();
};


struct Wire;
//struct WireCharge
//{
//	enum Action
//	{
//		INACTIVE,
//		RETRACTING,
//		//EXPLODING
//	};
//
//	WireCharge *next;
//	WireCharge *prev;
//	Action action;
//	Wire *wire;
//	V2d position;
//	int edgeIndex;
//	double edgeQuantity;
//	CollisionBox hitbox;
//	int vaIndex;
//
//	WireCharge(Wire *w, int vIndex);
//	void Reset();
//	void UpdatePrePhysics();
//	void UpdatePhysics();
//	void UpdatePostPhysics();
//	void UpdateSprite();
//	void ClearSprite();
//	void HitEnemy();
//};

struct SaveWireInfo;
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

	struct MyData
	{
		float shaderOffset;
		bool foundPoint;
		WirePoint anchor;
		WirePoint points[MAX_POINTS];
		WireState state;
		sf::Vector2i offset;
		V2d fireDir;
		int framesFiring;
		int frame;
		int numPoints;
		V2d realAnchor;
		bool canRetractGround;
		V2d closestPoint;
		double closestDiff;
		int fusePointIndex;
		V2d oldPos;
		V2d storedPlayerPos;
		V2d retractPlayerPos;
		V2d currOffset;
		V2d hitEnemyDelta;
		V2d anchorVel;
		V2d quadOldPosA;
		V2d quadOldWirePosB;
		V2d quadWirePosC;
		V2d quadPlayerPosD;
		double fuseQuantity;
		double minSideOther;
		double minSideAlong;
		double totalLength;
		double segmentLength;
		double minSegmentLength;
		double pullStrength;
		double dragStrength;
		int hitEnemyFrame;
		int hitEnemyFramesTotal;
		int firingTakingUp;
		int numVisibleIndexes;
		int newWirePoints;
		int aimingPrimaryAngleRange;
		int hitStallCounter;
		int antiWireGrassCount;
		CollisionBox movingHitbox;
		bool clockwise;
		double rcCancelDist;

		EdgeInfo rcEdge; //used in save states only
		double rcQuant; //used in save states only
	};
	
	MyData data;
	
	Edge *minSideEdge;
	
	
	Actor *player;
	int hitStallFrames;
	double retractSpeed;
	double maxTotalLength;
	double maxFireLength;
	double quadHalfWidth;
	double fireRate;
	double pullAccel;
	double maxPullStrength;
	double startPullStrength;
	double dragAccel;
	double maxDragStrength;
	double startDragStrength;
	double grassCheckRadius;
	
	sf::Shader wireShader;
	int extraBuffer; //when swinging around edges sometimes the wire
					 //stretches some. This is attemping to hole up that problem. if it happens
					 //too much then I can go into it and solve the real problems.
	int numAnimFrames;
	bool right;
	bool triggerDown;
	bool prevTriggerDown;
	std::string queryMode;
	std::list<sf::Drawable*> progressDraw;
	int numMinimapQuads;
	int numQuadVertices;
	int animFactor;
	sf::Sprite wireTip;
	sf::Sprite fuseSprite;
	Tileset *ts_wireTip;
	Tileset *ts_wire;
	Tileset *ts_miniHit;
	Tileset *ts_wireNode;
	HitboxInfo *tipHitboxInfo;
	sf::Vertex *quads;
	sf::Vertex *minimapQuads;
	sf::Vertex *nodeQuads;

	Wire( Actor *player, bool right );
	~Wire();
	void PopulateWireInfo(
		Wire::MyData *wi);
	void PopulateFromWireInfo(
		Wire::MyData *wi );
	void UpdateAnchors( V2d vel );
	void UpdateEnemyAnchor();
	bool TryFire();
	void SetCanRetractGround();
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
	void CheckAntiWireGrass();
	void TestPoint2( Edge *e );
	double GetTestPointAngle( Edge *e );
	void Retract();
	CollisionBox *GetTipHitbox();
	void SortNewPoints();
	bool IsPulling();
	bool IsRetracting();
	void SetStoredPlayerPos(const V2d &p);
	bool IsHit();

	//Rollback
	/*struct MyData : StoredEnemyData
	{
		int fireCounter;
	};*/

	/*int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);*/
};

#endif