#include <SFML/Graphics.hpp>
#include "VectorMath.h"
#include "QuadTree.h"
#include <list>
#include "nlohmann\json.hpp"

#ifndef __EDGE_H__
#define __EDGE_H__
#define NUM_STEPS 2.0
#define NUM_MAX_STEPS 10.0
#define MAX_VELOCITY 100

struct TerrainPolygon;
struct TerrainRail;
struct GameSession;
struct EdgeParentNode;
struct Gate;
struct Edge;

struct EdgeInfo
{
	enum EdgeInfoType
	{
		ETI_EMPTY,
		ETI_POLY,
		ETI_RAIL,
		ETI_GATE, //probably needed
		ETI_BORDER,
		ETI_Count,
	};

	EdgeInfoType eiType;
	int ownerIndex; //poly or rail
	int edgeIndex;

	EdgeInfo();
	void Clear();
	void SetFromEdge(Edge *e);
};

struct Edge : QuadTreeEntrant
{
	enum EdgeType
	{
		REGULAR,
		GATE,
		BORDER,
		BARRIER,
		Count
	};

	Edge();
	bool IsInvisibleWall();
	V2d GetReflectionDir(V2d &dir);
	V2d Normal();
	V2d Along();
	V2d FullAlong();
	V2d GetPosition( double quantity );
	V2d GetRaisedPosition(double quantity, double height );
	double GetQuantity( V2d &p );
	double GetRawQuantity(V2d &p);
	double GetQuantityGivenX( double x );
	double GetDistAlongNormal(V2d &p);
	double GetNormalAngleRadians();
	double GetNormalAngleDegrees();
	bool IsFlatGround();
	bool IsSlopedGround();
	bool IsSteepGround();
	bool IsWall(); 
	static double GetSteepThresh() { return .4; }
	//double GetNormalAngleRadians();
	//double GetNormalAngleDegrees();

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );
	double GetLength();
	double GetLengthSqr();
	void CalcAABB();

	bool IsLockedGateEdge();
	bool IsUnlockedGateEdge();
	bool IsOpenGateEdge();
	bool IsGateEdge();
	Gate *GetGate();

	V2d v0;
	V2d v1;
	Edge * GetPrevEdge();
	Edge * GetNextEdge();
	Edge *edge0;
	Edge *edge1;
	EdgeType edgeType;
	TerrainPolygon *poly;
	TerrainRail *rail;
	sf::Rect<double> aabb;
	int edgeIndex;//only when finalized for gameplay does this take on its real value atm
	bool secretZoneEdge; //to avoid drawing lines at the preview

	void *info;
};

struct HitboxInfo
{
	enum HitboxType
	{
		NORMAL,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		GREY,
		BLACK,
		WIREHITRED,
		WIREHITBLUE,
		SCORPIONSTRIKE,
		NO_HITBOX,
		Count
	};

	enum HitPosType
	{
		GROUND,
		GROUNDLOW,
		GROUNDHIGH,
		AIRDOWN,
		AIRDOWNFORWARD,
		AIRFORWARD,
		AIRUPFORWARD,
		AIRUP,
		OMNI,
	};

	HitboxInfo()
		/*:knockback(0),
		drainX(0), drainY(0), hitstunFrames(1), hitlagFrames(1), damage(10),
		freezeDuringStun(false), hType(NORMAL), hitToSidePlayerIsOn(true),
		invincibleFrames( -1 ), gravMultiplier( 1.0 ),
		extraDefenderHitlag( 0 ),
		hitBlockCancelDelay( 0 ),
		hitPosType( AIRFORWARD ),
		canBeBlocked(true),
		canBeParried(true),
		hitsThroughInvincibility(false),
		flipHorizontalKB( false ),
		flipVerticalKB( false ),
		reversableKnockback(true ),
		sensor(false)*/
	{
		Clear();
		kbDir = normalize(V2d(1, -1));
		//gravMultiplier = .5;
	};

	void Clear()
	{
		knockback = 0;
		drainX = 0;
		drainY = 0;
		hitstunFrames = 1;
		hitlagFrames = 1;
		damage = 10;
		freezeDuringStun = false;
		hType = NORMAL;
		hitToSidePlayerIsOn = true;
		invincibleFrames = -1;
		gravMultiplier = 1.0;
		extraDefenderHitlag = 0;
		hitBlockCancelDelay = 0;
		hitPosType = AIRFORWARD;
		canBeBlocked = true;
		canBeParried = true;
		hitsThroughInvincibility = false;
		flipHorizontalKB = false;
		flipVerticalKB = false;
		reversableKnockback = true;
		sensor = false;
		comboer = false;
		knockbackOnBlock = true;
	}

	static bool IsAirType(HitPosType hpt);
	static void SetupHitboxLevelInfo(
		nlohmann::json &j, 
		HitboxInfo &hi);
	static HitPosType GetAirType(const V2d &dir);

	V2d hDir;
	HitboxType hType;
	double knockback; //0+
	V2d GetKnockbackDir();
	V2d GetKnockbackVector();
	bool IsEmpty();
	void SetEmpty();
	V2d kbDir;
	//double drain; //0-1
	double drainX;
	double drainY;
	int hitstunFrames;
	int hitlagFrames;
	int damage;
	bool freezeDuringStun;
	bool hitToSidePlayerIsOn;
	int invincibleFrames;
	double gravMultiplier;
	int extraDefenderHitlag;
	int hitBlockCancelDelay;
	HitPosType hitPosType;
	bool canBeBlocked;
	bool canBeParried;
	bool hitsThroughInvincibility;
	bool flipHorizontalKB;
	bool flipVerticalKB;
	bool reversableKnockback;
	bool sensor; //doesn't register as recent hitter
	bool comboer;
	bool knockbackOnBlock;
	
};

struct CollisionBox
{
	enum BoxType
	{
		Physics,
		Hit,
		Hurt,
		Slow
	};

	enum Shape
	{

	};

	CollisionBox()
		:globalAngle( 0 ), rw( 0 ), rh( 0 ), isCircle( true ),
		flipHorizontal( false ), flipVertical( false )
		, localAngle( 0 ), isRing( false ), innerRadius(0)
	{

	}

	/*CollisionBox( HitboxInfo *hInfo)
		:globalAngle(0), rw(0), rh(0), isCircle(true),
		flipHorizontal(false), flipVertical(false),
		localAngle( 0 )
	{

	}*/
	bool Intersects( CollisionBox &c );
	V2d GetQuadVertex(int index);
	//double offsetAngle;
	V2d globalPosition;
	void Scale(double factor);
	void Move(V2d &move);
	double globalAngle;

	void SetRectDir(V2d &dir, double alongSize,
		double otherSize);
	double localAngle;
	V2d GetOffset();
	V2d GetTrueCenter();

	V2d offset;
	void DebugDraw( CollisionBox::BoxType bType, sf::RenderTarget *target );
	sf::Rect<double> GetAABB();

	double rw; //radius or half width
	double rh; //radius or half height
	bool isCircle;
	bool flipHorizontal;
	bool flipVertical;

	bool isRing;
	double innerRadius;
	//BoxType type;
};

struct CollisionBody
{
	int bodyID;

	CollisionBody();
	CollisionBody(CollisionBox::BoxType bType );
	CollisionBody(int p_numFrames, std::map<int, 
		std::list<CollisionBox>> & hList,
		HitboxInfo *hInfo );

	void BasicSetup(); //one frame and one hitbox
	void BasicCircleSetup(double radius,
		double a, V2d &offset );
	void BasicCircleSetup(double radius);
	void BasicCircleSetup(double radius,
		double a, V2d &offset, V2d &pos);
	void BasicCircleSetup(double radius,
		V2d &pos);
	void SetBasicPos(V2d &pos);
	void SetBasicPos(V2d &pos, double angle);
	void SetBasicPos( int frame, V2d &pos, double angle);
	V2d GetBasicPos();
	int GetFirstNonEmptyFrame();

	void BasicRectSetup(double w, double h, double a,
		V2d &offset);

	void SetupNumFrames(int p_numFrames);
	void SetupNumBoxesOnFrame(int frame, int numBoxes);

	~CollisionBody();
	void ResetFrames();

	bool IsEmpty();

	void Move(V2d &move);

	void DebugDraw( int frame, sf::RenderTarget *target);
	
	void AddBasicCircle( int frame, double radius, double angle, 
		V2d &offset);
	void SetLastPos( int frame, V2d &pos);
	void AddBasicRect( int frmae, double hw, double hh,
		double angle, V2d &offset);
	//void AddBasicRect(  )

	std::vector<CollisionBox> &GetCollisionBoxes(int frame);
	void AddCollisionBox(int frame, CollisionBox &cb);
	int GetNumFrames() { return numFrames; }
	int GetNumBoxes(int frame);
	sf::Rect<double> GetAABB( int frame );
	bool Intersects( int frame, CollisionBody *other, 
		int otherFrame );
	void OffsetFrame(int frame, sf::Vector2f &offset);
	void OffsetAllFrames(sf::Vector2f &offset);
	HitboxInfo *hitboxInfo;
	bool Intersects( int frame, CollisionBox *box);
	CollisionBox::BoxType boxType;
private:
	int numFrames;
	
	//BoxType type;
	std::vector<std::vector<CollisionBox>> collisionBoxVectors;
};

struct Contact
{
	Contact();
	void Reset();
		
	double collisionPriority;	
	V2d position;
	V2d resolution;
	Edge *edge;
	V2d normal;
	bool weirdPoint;

};

struct Collider
{
	Collider();
	~Collider();
	
	Contact *collideEdge( 
		V2d position, 
		const CollisionBox &b, Edge *e, 
		const V2d &vel, 
		const V2d &tVel );
	
	void DebugDraw( sf::RenderTarget *target );
	void ClearDebug();

	Contact *currentContact;
	std::list<sf::Drawable*> progressDraw;

};

struct EdgeQNode
{
	EdgeQNode():parent(NULL),debug(NULL){}
	V2d pos;
	double rw;
	double rh;
	sf::RenderWindow *debug;
	EdgeParentNode *parent;
	bool leaf;
};

struct EdgeParentNode : EdgeQNode
{
	EdgeParentNode( const V2d &pos, double rw, double rh );
	EdgeQNode *children[4];
	// 0    |     1
	//--------------
	// 2    |     3
	
};

struct EdgeLeafNode : EdgeQNode
{
	int objCount;
	EdgeLeafNode( const V2d &pos, double rw, double rh );
	Edge *edges[4];
};

EdgeQNode *Insert( EdgeQNode *node, Edge* e );

void DebugDrawQuadTree( sf::RenderWindow *rw, EdgeQNode *node );

struct EdgeQuadTreeCollider
{
	virtual void HandleEdge( Edge *e ) = 0;
};

void Query( EdgeQuadTreeCollider *qtc, EdgeQNode *node, const sf::Rect<double> &r );

bool IsEdgeTouchingBox( Edge *e, const sf::Rect<double> & ir );

struct RayCastInfo
{
	Edge * rcEdge;
	double rcQuant;
	double rcPortion;
	V2d rayStart;
	V2d rayEnd;
	QuadTree *tree;

	RayCastInfo()
	{
		tree = NULL;
		Reset();
	}
	void Reset()
	{
		rcEdge = NULL;
		rcQuant = 0;
		rcPortion = 0;
		rayStart.x = 0;
		rayStart.y = 0;
		rayEnd.x = 0;
		rayEnd.y = 0;
	}
	V2d GetRayHitPos()
	{
		if (rcEdge != NULL)
		{
			return rcEdge->GetPosition(rcQuant);
		}

		return V2d();
	}
};

struct RayCastHandler
{
	RayCastInfo rayCastInfo;
	
	bool ExecuteRayCast( V2d &start, V2d& end );
	
	virtual void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
};

void RayCast( RayCastHandler *handler, 
	QNode *node, 
	V2d &startPoint, 
	V2d &endPoint );

void RayCast(RayCastHandler *handler,
	QNode *node,
	Edge &e);

bool IsBoxTouchingBox( const sf::Rect<double> & r0, 
	const sf::Rect<double> & r1 );
bool IsBoxTouchingBox( const sf::Rect<int> & r0, 
	const sf::Rect<int> & r1 );

bool isQuadTouchingQuad( V2d & A0, 
						 V2d & B0, 
						 V2d & C0, 
						 V2d & D0, 
						 V2d & A1, 
						 V2d & B1, 
						 V2d & C1, 
						 V2d & D1 );

bool IsQuadTouchingCircle(V2d & A, 
						  V2d &B, 
						  V2d &C, 
						  V2d &D, 
						  V2d &pos, 
						  double rad );

bool QuadContainsPoint( V2d &A, 
						V2d &B, 
						V2d &C, 
						V2d &D, 
						V2d &point );

sf::FloatRect GetQuadAABB(sf::Vertex *v);


sf::Vector2f GetQuadCenter(sf::Vertex *v);

bool QuadContainsPoint(
	sf::Vector2f &A,
	sf::Vector2f &B,
	sf::Vector2f &C,
	sf::Vector2f &D,
	sf::Vector2f &point);

bool QuadContainsPoint(sf::Vertex *quad, 
	sf::Vector2f &point);

bool IsEdgeTouchingCircle( 
	V2d & v0, 
	V2d &v1, 
	V2d &pos, double rad );

bool IsEdgeTouchingQuad(V2d &v0,
	V2d &v1,
	V2d &A,
	V2d &B,
	V2d &C,
	V2d &D);

bool WithinDistance( V2d &A, V2d &B, double rad);


//struct QuadTree
//{
//};
#endif