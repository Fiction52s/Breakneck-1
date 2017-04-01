#ifndef __WIRE_H__
#define __WIRE_H__

#include <SFML/Graphics.hpp>
//#include "Actor.h"
#include "Physics.h"
#include "QuadTree.h"
#include "Tileset.h"
#include <SFML/System/Clock.hpp>
//#include "GameSession.h"

struct Actor;
struct Wire : RayCastHandler, QuadTreeCollider
{
	enum WireState
	{
		IDLE,
		FIRING,
		HIT,
		PULLING,
		RETRACTING,
		RELEASED
	};

	sf::Clock wireTestClock;
	//sf::Vector2<double> 
	Wire( Actor *player, bool right );
	void UpdateAnchors( sf::Vector2<double> vel );
	void UpdateAnchors2( sf::Vector2<double> vel );
	void SetFireDirection( sf::Vector2<double> dir );
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
	sf::Vector2<double> GetOriginPos( bool test );
	void UpdateFuse();
	double GetSegmentLength();
	sf::Vector2<double> GetPlayerPos();
	sf::Vector2<double> currOffset;

	double GetCurrentTotalLength();
	
	int extraBuffer; //when swinging around edges sometimes the wire
	//stretches some. This is attemping to hole up that problem. if it happens
	//too much then I can go into it and solve the real problems.
	int numAnimFrames;
	bool foundPoint;
	sf::Vector2<double> closestPoint;
	Tileset *ts_wire;
	Tileset *ts_miniHit;
	int firingTakingUp;
	//Tileset *ts_redWire;
	//double closestInfo;
	double closestDiff;
	sf::Vector2<double> realAnchor;
	sf::Vector2<double> oldPos;
	bool clockwise;
	sf::Vector2<double> storedPlayerPos;


	sf::Vector2i offset;
	int addedPoints;
	bool right;
	WireState state;

	sf::Vector2<double> retractPlayerPos;
	double retractSpeed;
	int fusePointIndex;
	double fuseQuantity;
	sf::Sprite fuseSprite;
	bool canRetractGround;

	bool triggerDown;
	bool prevTriggerDown;

	int numVisibleIndexes;

	struct WirePoint
	{
		Edge *e;
		double quantity;
		bool start;
		sf::Vector2<double> pos;
		sf::Vector2<double> edgeEnd;
		sf::Vector2<double> test;
		bool clockwise;
		double angleDiff;

		double sortingAngleDist;
	};

	const static int MAX_CHARGES = 16;
	struct WireCharge
	{
		enum Action
		{
			INACTIVE,
			RETRACTING,
			//EXPLODING
		};

		WireCharge( Wire *w, int vIndex );
		void Reset();
		void UpdatePrePhysics();
		void UpdatePhysics();
		void UpdatePostPhysics();
		void UpdateSprite();
		void ClearSprite();

		void HitEnemy();
		WireCharge *next;
		WireCharge *prev;
		Action action;
		Wire *wire;
		sf::Vector2<double> position;
		int edgeIndex;
		double edgeQuantity;
		CollisionBox hitbox;
		int vaIndex;
	};

	void DrawWireCharges( sf::RenderTarget *target );
	WireCharge *activeChargeList;
	WireCharge *inactiveChargeList;
	void CreateWireCharge();
	void DeactivateWireCharge( WireCharge *wc );
	WireCharge * GetWireCharge();
	void ActivateWireCharge( int index );
	void ClearCharges();
	void ActivateRetractionCharges();
	void UpdateChargesPhysics();
	void UpdateChargesSprites();
	void UpdateChargesPrePhysics();
	void UpdateChargesPostPhysics();

	void TestPoint2( Edge *e );
	int newWirePoints;
	double GetTestPointAngle( Edge *e );

	Tileset *ts_wireTip;
	sf::Sprite wireTip;

	int CountActiveCharges();
	int CountInactiveCharges();
	Tileset *ts_wireCharge;
	
	int numTotalCharges;
	sf::VertexArray chargeVA;


	double maxTotalLength;
	double maxFireLength;

	int frame;
	int animFactor;
	double quadHalfWidth;
	int numPoints;
	const static int MAX_POINTS = 64;

	//sf::Vector2<double> points[16];
	WirePoint points[MAX_POINTS];

	void SortNewPoints( );
	//sf::VertexArray quads;
	sf::Vertex *quads;
	int numQuadVertices;
	//sf::VertexArray minimapQuads;
	sf::Vertex *minimapQuads;
	int numMinimapQuads;
	int framesFiring;
	double fireRate;
	Edge *minSideEdge;
	double minSideOther;
	double minSideAlong;
	sf::Vector2<double> fireDir;
	WirePoint anchor;

	void ActivateCharges();
	void Retract();

	int triggerThresh;
	int hitStallFrames;
	int hitStallCounter;

	double totalLength;
	//double minTotalLength;
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

	Actor *player;

	Edge *rcEdge;
	double rcQuant;

	bool rayCancel;

	sf::Vector2<double> quadOldPosA;
	sf::Vector2<double> quadOldWirePosB;
	sf::Vector2<double> quadWirePosC;
	sf::Vector2<double> quadPlayerPosD;
	sf::Vector2<double> minPoint;


	std::list<sf::Drawable*> progressDraw;
};

#endif