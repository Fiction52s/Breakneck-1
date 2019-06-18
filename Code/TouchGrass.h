#ifndef __TOUCHGRASS_H__
#define __TOUCHGRASS_H__

#include "QuadTree.h"
#include "Physics.h"

struct TerrainPiece;
struct Edge;
struct Tileset;
struct Actor;


struct TouchGrass : QuadTreeEntrant
{
	enum TouchGrassType
	{
		TYPE_NORMAL
	};

	enum Action
	{
		STILL,
		TOUCHEDLEFT,
		TOUCHEDRIGHT,
		TOUCHEDLAND,
	};


	TouchGrass(TouchGrassType type, int index, sf::Vertex *va,
		Tileset *ts, Edge *e, double quant,
		double rad, double yOff);
	void Reset();
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	void Update();
	void Touch( Actor *a);
	void Destroy(Actor *a);
	bool Intersects(CollisionBody *cb, int frame);
	bool visible;

	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	HitboxInfo *hitboxInfo;
	
	V2d center;
	float angle;
	Action action;
	TouchGrassType gType;
	Edge *edge;
	double quant;
	int gIndex;
	TerrainPiece *poly;
	double yOffset;
	double radius;
	Tileset *ts;
	sf::Vertex *va;
	sf::Vertex *myQuad;
	int frame;
	void UpdateSprite();
	sf::Vector2<double> points[4];
};

struct QuadTree;
struct TouchGrassCollection
{
	TouchGrassCollection();
	void Reset();
	~TouchGrassCollection();
	QuadTree * touchGrassTree;
	sf::Vertex *touchGrassVA;
	int numTouchGrasses;
	void Draw(sf::RenderTarget *target);
	void Query(QuadTreeCollider *qtc, sf::Rect<double> &r);
	void UpdateGrass();
	std::list<TouchGrass*> myGrass;
	Tileset *ts_grass;
};

#endif