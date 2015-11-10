#ifndef __WIRE_H__
#define __WIRE_H__

#include <SFML/Graphics.hpp>
//#include "Actor.h"
#include "Physics.h"
#include "QuadTree.h"
#include "Tileset.h"
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
		RELEASED
	};

	//sf::Vector2f 
	Wire( Actor *player, bool right );
	void UpdateAnchors( sf::Vector2f vel );
	void UpdateAnchors2();
	void SetFireDirection( sf::Vector2f dir );
	void Check();
	void HandleRayCollision( Edge *edge, float edgeQuantity, float rayPortion );
	void UpdateState( bool touchEdgeWithWire );
	void Draw( sf::RenderTarget *target );
	void DebugDraw( sf::RenderTarget *target );
	void ClearDebug();
	void HandleEntrant( QuadTreeEntrant *qte );
	void TestPoint( Edge * e);
	void SortNewPoints( int start, int end );
	void SwapPoints( int aIndex, int bIndex );
	void UpdateQuads();
	void Reset();
	sf::Vector2f GetOriginPos( bool test );
	
	int extraBuffer; //when swinging around edges sometimes the wire
	//stretches some. This is attemping to hole up that problem. if it happens
	//too much then I can go into it and solve the real problems.

	bool foundPoint;
	sf::Vector2f closestPoint;
	Tileset *ts_wire;
	//Tileset *ts_redWire;
	//float closestInfo;
	float closestDiff;
	sf::Vector2f realAnchor;
	sf::Vector2f oldPos;
	bool clockwise;
	sf::Vector2f storedPlayerPos;

	sf::Vector2i offset;
	int addedPoints;
	bool right;
	WireState state;

	struct WirePoint
	{
		Edge *e;
		float quantity;
		bool start;
		sf::Vector2f pos;
		sf::Vector2f edgeEnd;
		sf::Vector2f test;
		bool clockwise;
		float angleDiff;
	};

	float maxTotalLength;

	int frame;
	int animFactor;
	float quadHalfWidth;
	int numPoints;
	const static int MAX_POINTS = 20000;
	//sf::Vector2f points[16];
	WirePoint points[MAX_POINTS];
	sf::VertexArray quads;
	int framesFiring;
	float fireRate;
	Edge *minSideEdge;
	float minSideOther;
	float minSideAlong;
	sf::Vector2f fireDir;
	WirePoint anchor;
	

	int triggerThresh;
	int hitStallFrames;
	int hitStallCounter;

	float totalLength;
	//float minTotalLength;
	float segmentLength;
	float minSegmentLength;
	float pullStrength;

	Actor *player;

	Edge *rcEdge;
	float rcQuant;

	sf::Vector2f quadOldPosA;
	sf::Vector2f quadOldWirePosB;
	sf::Vector2f quadWirePosC;
	sf::Vector2f quadPlayerPosD;
	sf::Vector2f minPoint;

	std::list<sf::Drawable*> progressDraw;
};

#endif