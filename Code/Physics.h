#include <SFML/Graphics.hpp>
#include "VectorMath.h"
#include "QuadTree.h"
#include <list>

#ifndef __EDGE_H__
#define __EDGE_H__
#define NUM_STEPS 2.0
#define NUM_MAX_STEPS 10.0
#define MAX_VELOCITY 100

struct TerrainPiece;

struct Edge : QuadTreeEntrant
{
	enum EdgeType
	{
		REGULAR,
		OPEN_GATE,
		CLOSED_GATE,
		BORDER,
		Count
	};

	Edge();
	sf::Vector2<double> Normal();
	sf::Vector2<double> Along();
	sf::Vector2<double> GetPoint( double quantity );
	double GetQuantity( sf::Vector2<double> p );
	double GetQuantityGivenX( double x );

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );
	double GetLength();
	sf::Vector2<double> v0;
	sf::Vector2<double> v1;
	Edge * GetEdge0();
	Edge * GetEdge1();
	Edge *edge0;
	Edge *edge1;
	EdgeType edgeType;
	TerrainPiece *poly;


	void *info;
};

struct GameSession;
struct MovingTerrain
{
	MovingTerrain( GameSession *owner, sf::Vector2i pos, 
		std::list<sf::Vector2i> &pathParam, 
		std::list<sf::Vector2i> &pointsParam,
		bool loop, float speed );
	~MovingTerrain();
	void AddPoint(sf::Vector2i p);
	void Finalize();
	void Query( QuadTreeCollider *qtc, const sf::Rect<double> &r );
	void DebugDraw( sf::RenderTarget *target );
	void Draw( sf::RenderTarget *target );
	void UpdatePhysics();
	void AdvanceTargetNode();
	//std::list<Vector2i> tempPoints;
	QuadTree *quadTree;
	Edge **edgeArray;
	sf::VertexArray *polygonVA;
	int numEdges;
	int left;
	int right;
	int top;
	int bottom;
	sf::Vector2<double> position;
	sf::Vector2<double> oldPosition;
	sf::Vector2i *path; //global coords
	int pathLength;
	bool loop;
	sf::Vector2<double> vel;
	int slowMultiple;
	int slowCounter;
	int targetNode;
	bool forward;
	double speed;
	GameSession *owner;
	int numTris;
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

	CollisionBox( BoxType bType = BoxType::Hit )
		:globalAngle( 0 ), rw( 0 ), rh( 0 ), isCircle( true ), type(bType),
		flipHorizontal( false ), flipVertical( false )
		, localAngle( 0 )
	{

	}

	CollisionBox( HitboxInfo *hInfo, BoxType bType = BoxType::Hit )
		:globalAngle(0), rw(0), rh(0), isCircle(true), type(bType),
		flipHorizontal(false), flipVertical(false),
		localAngle( 0 )
	{

	}
	bool Intersects( CollisionBox &c );
	sf::Vector2<double> GetQuadVertex(int index);
	//double offsetAngle;
	sf::Vector2<double> globalPosition;
	void Scale(double factor);
	double globalAngle;

	void SetRectDir(V2d &dir, double alongSize,
		double otherSize);
	double localAngle;
	V2d GetOffset();
	V2d GetTrueCenter();

	sf::Vector2<double> offset;
	void DebugDraw( sf::RenderTarget *target );
	sf::Rect<double> GetAABB();

	double rw; //radius or half width
	double rh; //radius or half height
	bool isCircle;
	bool flipHorizontal;
	bool flipVertical;
	BoxType type;
};

struct CollisionBody
{
	CollisionBody(int p_numFrames);
	CollisionBody(int p_numFrames, std::map<int, std::list<CollisionBox>> & hList,
		HitboxInfo *hInfo );
	~CollisionBody();

	void DebugDraw( int frame, sf::RenderTarget *target);
	
	std::list<CollisionBox> *GetCollisionBoxes(int frame);
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
private:
	int numFrames;
	
	//BoxType type;
	std::list<CollisionBox> **collisionBoxLists;
};

struct Hitbox : CollisionBox
{
	Hitbox() 
		:CollisionBox( BoxType::Hit )
	{}
};



struct Contact
{
	Contact();
		
	double collisionPriority;	
	sf::Vector2<double> position;
	sf::Vector2<double> resolution;
	Edge *edge;
	sf::Vector2<double> normal;
	MovingTerrain *movingPlat;
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

bool WithinDistance(sf::Vector2<double> &A,
	sf::Vector2<double> &B, double rad);


//struct QuadTree
//{
//};
#endif