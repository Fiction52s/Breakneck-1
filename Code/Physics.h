#include <SFML/Graphics.hpp>
#include "VectorMath.h"
#include "QuadTree.h"
#include <list>

#ifndef __EDGE_H__
#define __EDGE_H__

struct Edge : QuadTreeEntrant
{
	Edge();
	sf::Vector2f Normal();
	sf::Vector2f GetPoint( float quantity );
	float GetQuantity( sf::Vector2f p );
	float GetQuantityGivenX( float x );

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<float> &r );

	sf::Vector2f v0;
	sf::Vector2f v1;
	Edge * GetEdge0();
	Edge * GetEdge1();
	Edge *edge0;
	Edge *edge1;
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
	void Query( QuadTreeCollider *qtc, const sf::Rect<float> &r );
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
	sf::Vector2f position;
	sf::Vector2f oldPosition;
	sf::Vector2i *path; //global coords
	int pathLength;
	bool loop;
	sf::Vector2f vel;
	int slowMultiple;
	int slowCounter;
	int targetNode;
	bool forward;
	float speed;
	GameSession *owner;
	int numTris;
};

struct CollisionBox
{
	enum BoxType
	{
		Physics,
		Hit,
		Hurt
	};

	
	bool Intersects( CollisionBox &c );
	//float offsetAngle;
	sf::Vector2f globalPosition;
	float globalAngle;

	sf::Vector2f offset;
	void DebugDraw( sf::RenderTarget *target );

	float rw; //radius or half width
	float rh; //radius or half height
	bool isCircle;
	BoxType type;	
};

struct HitboxInfo
{
	float knockback; //0+
	float drain; //0-1
	int hitstunFrames; 
	int hitlagFrames;
	int damage;
};

struct Contact
{
	Contact();
		
	float collisionPriority;	
	sf::Vector2f position;
	sf::Vector2f resolution;
	Edge *edge;
	sf::Vector2f normal;
	MovingTerrain *movingPlat;
	bool weirdPoint;

};

struct Collider
{
	Collider();
	~Collider();
	
	Contact *collideEdge( 
		sf::Vector2f position, 
		const CollisionBox &b, Edge *e, 
		const sf::Vector2f &vel, 
		const sf::Vector2f &tVel );
	
	void DebugDraw( sf::RenderTarget *target );
	void ClearDebug();

	Contact *currentContact;
	std::list<sf::Drawable*> progressDraw;

};

struct EdgeParentNode;

struct EdgeQNode
{
	EdgeQNode():parent(NULL),debug(NULL){}
	sf::Vector2f pos;
	float rw;
	float rh;
	sf::RenderWindow *debug;
	EdgeParentNode *parent;
	bool leaf;
};



struct EdgeParentNode : EdgeQNode
{
	EdgeParentNode( const sf::Vector2f &pos, float rw, float rh );
	EdgeQNode *children[4];
	// 0    |     1
	//--------------
	// 2    |     3
	
};

struct EdgeLeafNode : EdgeQNode
{
	int objCount;
	EdgeLeafNode( const sf::Vector2f &pos, float rw, float rh );
	Edge *edges[4];
};

EdgeQNode *Insert( EdgeQNode *node, Edge* e );
//void Query( EdgeQNode *node, void (*f)( Edge *e ) );

void DebugDrawQuadTree( sf::RenderWindow *rw, EdgeQNode *node );

struct EdgeQuadTreeCollider
{
	virtual void HandleEdge( Edge *e ) = 0;
};

void Query( EdgeQuadTreeCollider *qtc, EdgeQNode *node, const sf::Rect<float> &r );

bool IsEdgeTouchingBox( Edge *e, const sf::Rect<float> & ir );

struct RayCastHandler
{
	virtual void HandleRayCollision( Edge *edge, float edgeQuantity, float rayPortion ) = 0;
};

void RayCast( RayCastHandler *handler, QNode *node, 
	sf::Vector2f startPoint, 
	sf::Vector2f endPoint );

bool IsBoxTouchingBox( const sf::Rect<float> & r0, const sf::Rect<float> & r1 );

bool isQuadTouchingQuad(  sf::Vector2f & A0, 
						 sf::Vector2f & B0, 
						 sf::Vector2f & C0, 
						 sf::Vector2f & D0, 
						 sf::Vector2f & A1, 
						 sf::Vector2f & B1, 
						 sf::Vector2f & C1, 
						 sf::Vector2f & D1 );
//struct QuadTree
//{
//};
#endif