#include <SFML/Graphics.hpp>
#include "VectorMath.h"
#include "QuadTree.h"
#include <list>

#ifndef __EDGE_H__
#define __EDGE_H__
#define NUM_STEPS 2.0
#define NUM_MAX_STEPS 10.0
#define MAX_VELOCITY 100

struct TerrainPolygon;

struct Edge : QuadTreeEntrant
{
	enum EdgeType
	{
		REGULAR,
		OPEN_GATE,
		CLOSED_GATE,
		BORDER,
		BARRIER,
		Count
	};

	Edge();
	bool IsInvisibleWall();
	sf::Vector2<double> Normal();
	sf::Vector2<double> Along();
	sf::Vector2<double> FullAlong();
	sf::Vector2<double> GetPosition( double quantity );
	sf::Vector2<double> GetRaisedPosition(double quantity, double height );
	double GetQuantity( sf::Vector2<double> &p );
	double GetRawQuantity(sf::Vector2<double> &p);
	double GetQuantityGivenX( double x );
	double GetDistAlongNormal(sf::Vector2<double> &p);
	double GetNormalAngleRadians();
	double GetNormalAngleDegrees();
	//double GetNormalAngleRadians();
	//double GetNormalAngleDegrees();

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );
	double GetLength();
	double GetLengthSqr();

	sf::Vector2<double> v0;
	sf::Vector2<double> v1;
	Edge * GetPrevEdge();
	Edge * GetNextEdge();
	Edge *edge0;
	Edge *edge1;
	EdgeType edgeType;
	TerrainPolygon *poly;

	void *info;
};

struct GameSession;

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
		WIREHITRED,
		WIREHITBLUE,
		SCORPIONSTRIKE,
		COMBO,
		Count
	};

	HitboxInfo()
		:knockback(0), kbDir(0, 0),
		drainX(0), drainY(0), hitstunFrames(1), hitlagFrames(1), damage(10),
		freezeDuringStun(false), hType(NORMAL), hitToSidePlayerIsOn(true)
	{
	};

	sf::Vector2<double> hDir;
	HitboxType hType;
	double knockback; //0+
	sf::Vector2<double> kbDir;
	//double drain; //0-1
	double drainX;
	double drainY;
	int hitstunFrames;
	int hitlagFrames;
	int damage;
	bool freezeDuringStun;
	bool hitToSidePlayerIsOn;

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
		, localAngle( 0 )
	{

	}

	/*CollisionBox( HitboxInfo *hInfo)
		:globalAngle(0), rw(0), rh(0), isCircle(true),
		flipHorizontal(false), flipVertical(false),
		localAngle( 0 )
	{

	}*/
	bool Intersects( CollisionBox &c );
	sf::Vector2<double> GetQuadVertex(int index);
	//double offsetAngle;
	sf::Vector2<double> globalPosition;
	void Scale(double factor);
	void Move(V2d &move);
	double globalAngle;

	void SetRectDir(V2d &dir, double alongSize,
		double otherSize);
	double localAngle;
	V2d GetOffset();
	V2d GetTrueCenter();

	sf::Vector2<double> offset;
	void DebugDraw( CollisionBox::BoxType bType, sf::RenderTarget *target );
	sf::Rect<double> GetAABB();

	double rw; //radius or half width
	double rh; //radius or half height
	bool isCircle;
	bool flipHorizontal;
	bool flipVertical;
	//BoxType type;
};

struct CollisionBody
{
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

	void BasicRectSetup(double w, double h, double a,
		V2d &offset);

	void SetupNumFrames(int p_numFrames);
	void SetupNumBoxesOnFrame(int frame, int numBoxes);

	~CollisionBody();
	void ResetFrames();

	bool Empty();

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
		
	double collisionPriority;	
	sf::Vector2<double> position;
	sf::Vector2<double> resolution;
	Edge *edge;
	sf::Vector2<double> normal;
	bool weirdPoint;

};

struct Collider
{
	Collider();
	~Collider();
	
	Contact *collideEdge( 
		sf::Vector2<double> position, 
		const CollisionBox &b, Edge *e, 
		const sf::Vector2<double> &vel, 
		const sf::Vector2<double> &tVel );
	
	void DebugDraw( sf::RenderTarget *target );
	void ClearDebug();

	Contact *currentContact;
	std::list<sf::Drawable*> progressDraw;

};

struct EdgeParentNode;

struct EdgeQNode
{
	EdgeQNode():parent(NULL),debug(NULL){}
	sf::Vector2<double> pos;
	double rw;
	double rh;
	sf::RenderWindow *debug;
	EdgeParentNode *parent;
	bool leaf;
};



struct EdgeParentNode : EdgeQNode
{
	EdgeParentNode( const sf::Vector2<double> &pos, double rw, double rh );
	EdgeQNode *children[4];
	// 0    |     1
	//--------------
	// 2    |     3
	
};

struct EdgeLeafNode : EdgeQNode
{
	int objCount;
	EdgeLeafNode( const sf::Vector2<double> &pos, double rw, double rh );
	Edge *edges[4];
};

EdgeQNode *Insert( EdgeQNode *node, Edge* e );
//void Query( EdgeQNode *node, void (*f)( Edge *e ) );

void DebugDrawQuadTree( sf::RenderWindow *rw, EdgeQNode *node );

struct EdgeQuadTreeCollider
{
	virtual void HandleEdge( Edge *e ) = 0;
};

void Query( EdgeQuadTreeCollider *qtc, EdgeQNode *node, const sf::Rect<double> &r );

bool IsEdgeTouchingBox( Edge *e, const sf::Rect<double> & ir );

struct RayCastHandler
{
	virtual void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion ) = 0;
};

void RayCast( RayCastHandler *handler, QNode *node, 
	sf::Vector2<double> startPoint, 
	sf::Vector2<double> endPoint );

bool IsBoxTouchingBox( const sf::Rect<double> & r0, const sf::Rect<double> & r1 );
bool IsBoxTouchingBox( const sf::Rect<int> & r0, const sf::Rect<int> & r1 );

bool isQuadTouchingQuad(  sf::Vector2<double> & A0, 
						 sf::Vector2<double> & B0, 
						 sf::Vector2<double> & C0, 
						 sf::Vector2<double> & D0, 
						 sf::Vector2<double> & A1, 
						 sf::Vector2<double> & B1, 
						 sf::Vector2<double> & C1, 
						 sf::Vector2<double> & D1 );

bool IsQuadTouchingCircle( sf::Vector2<double> & A, 
						  sf::Vector2<double> &B, 
						  sf::Vector2<double> &C, 
						  sf::Vector2<double> &D, 
						  sf::Vector2<double> &pos, 
						  double rad );

bool QuadContainsPoint( sf::Vector2<double> &A, 
						sf::Vector2<double> &B, 
						sf::Vector2<double> &C, 
						sf::Vector2<double> &D, 
						sf::Vector2<double> &point );

bool IsEdgeTouchingCircle( sf::Vector2<double> & v0, 
	sf::Vector2<double> &v1, 
	sf::Vector2<double> &pos, double rad );

bool IsEdgeTouchingQuad(sf::Vector2<double> &v0,
	sf::Vector2<double> &v1,
	sf::Vector2<double> &A,
	sf::Vector2<double> &B,
	sf::Vector2<double> &C,
	sf::Vector2<double> &D);

bool WithinDistance(sf::Vector2<double> &A,
	sf::Vector2<double> &B, double rad);


//struct QuadTree
//{
//};
#endif